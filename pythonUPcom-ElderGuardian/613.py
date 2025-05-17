import dash
from dash import html, dcc
from dash.dependencies import Input, Output
import plotly.graph_objs as go
import requests
import time
import json
from collections import deque
from datetime import datetime

# ================= 配置信息 =================
DEVICE_NAME = "temp01"
PRODUCT_ID = "8VORdlxrZT"
AUTHORIZATION_TOKEN = "version=2018-10-31&res=products%2F8VORdlxrZT%2Fdevices%2Ftemp01&et=2017881776&method=md5&sign=rvz5GSi9ehQQNAzv%2FTV6cw%3D%3D"

URL = f"https://iot-api.heclouds.com/thingmodel/query-device-property?product_id={PRODUCT_ID}&device_name={DEVICE_NAME}"


# ================= 数据获取与解析 =================
def get_onenet_data():
    headers = {
        'Authorization': AUTHORIZATION_TOKEN
    }
    try:
        response = requests.get(URL, headers=headers, timeout=5)
        if response.status_code == 200:
            return response.json()
        else:
            print(f"请求失败，状态码：{response.status_code}")
            return None
    except Exception as e:
        print(f"请求异常：{e}")
        return None


def parse_data(json_data):
    data_list = json_data.get("data", [])
    result = {}
    for item in data_list:
        identifier = item.get("identifier")
        value = item.get("value")
        result[identifier] = value
    return result


# ================= 初始化历史数据 =================
MAX_POINTS = 50
history = {
    "timestamp": deque(maxlen=MAX_POINTS),
    "CurrentTemperature": deque(maxlen=MAX_POINTS),
    "CurrentHumidity": deque(maxlen=MAX_POINTS),
    "Light_value": deque(maxlen=MAX_POINTS),
}

# ================= Dash 应用初始化 =================
app = dash.Dash(__name__)
app.title = "老人健康监测大屏"

app.layout = html.Div([
    html.H1("👵 老人健康监测系统 - 实时监控大屏", style={'textAlign': 'center', 'fontFamily': 'San Francisco'}),

    html.Div(id='last-update', style={'textAlign': 'right', 'margin': '10px', 'fontSize': '14px'}),

    html.Div([
        html.Div(id='temperature-card', className='card'),
        html.Div(id='humidity-card', className='card'),
        html.Div(id='light-card', className='card'),
        html.Div(id='pir-card', className='card'),
        html.Div(id='vibration-card', className='card'),
    ], style={'display': 'flex', 'justifyContent': 'space-around', 'flexWrap': 'wrap'}),

    html.Div([
        dcc.Graph(id='temperature-graph', className='graph'),
        dcc.Graph(id='humidity-graph', className='graph'),
        dcc.Graph(id='light-graph', className='graph'),
    ], style={'marginTop': '20px'}),

    dcc.Interval(id='interval-component', interval=5 * 1000, n_intervals=0)
], style={'padding': '20px', 'backgroundColor': '#f8f9fa'})


# ================= 卡片样式函数 =================
def generate_card(title, value, color='black'):
    return html.Div([
        html.H3(title),
        html.P(value, style={'fontSize': '24px', 'fontWeight': 'bold', 'color': color})
    ])


# ================= 回调函数 =================
@app.callback(
    [Output('temperature-card', 'children'),
     Output('humidity-card', 'children'),
     Output('light-card', 'children'),
     Output('pir-card', 'children'),
     Output('vibration-card', 'children'),
     Output('temperature-graph', 'figure'),
     Output('humidity-graph', 'figure'),
     Output('light-graph', 'figure'),
     Output('last-update', 'children')],
    Input('interval-component', 'n_intervals')
)
def update_data(n):
    json_data = get_onenet_data()
    parsed = parse_data(json_data) if json_data else {}

    # 提取数据
    temp = parsed.get("CurrentTemperature", "N/A")
    humid = parsed.get("CurrentHumidity", "N/A")
    light = parsed.get("Light_value", "N/A")
    pir = parsed.get("PIR", "N/A")
    vibration = parsed.get("Vibration", "N/A")

    # 更新时间
    current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    # PIR 状态判断
    pir_status = "活动中" if pir == "1" else "休息中"
    pir_color = 'red' if pir == "1" else 'green'

    # 振动状态判断
    vibration_status = "行为异常" if vibration == "1" else "行为正常"
    vibration_color = 'red' if vibration == "1" else 'green'

    # 更新历史数据
    timestamp = time.time()
    history['timestamp'].append(timestamp)
    history['CurrentTemperature'].append(float(temp) if temp != "N/A" else None)
    history['CurrentHumidity'].append(float(humid) if humid != "N/A" else None)
    history['Light_value'].append(float(light) if light != "N/A" else None)

    # 构建图表
    def build_graph(data_key, title, unit, color='blue'):
        return go.Figure(
            data=[go.Scatter(x=list(history['timestamp']), y=list(history[data_key]), mode='lines+markers', name=title,
                             line=dict(color=color))],
            layout=go.Layout(
                title=title,
                xaxis_title="时间戳",
                yaxis_title=f"{title} ({unit})",
                template='plotly_white',
                height=300
            )
        )

    return (
        generate_card("🌡️ 温度", f"{temp} °C"),
        generate_card("💧 湿度", f"{humid} %"),
        generate_card("☀️ 光照强度", f"{light} lx"),
        generate_card("👁️ PIR 状态", pir_status, pir_color),
        generate_card("🌀 振动状态", vibration_status, vibration_color),
        build_graph("CurrentTemperature", "温度变化", "°C", "#ff7f0e"),
        build_graph("CurrentHumidity", "湿度变化", "%", "#1f77b4"),
        build_graph("Light_value", "光照强度变化", "lx", "#2ca02c"),
        f"最后更新时间: {current_time}"
    )


# ================= 启动服务 =================
if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8050, debug=False)