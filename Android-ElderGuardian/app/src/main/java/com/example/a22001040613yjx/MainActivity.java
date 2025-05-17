package com.example.a22001040613yjx;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.Handler;
import android.os.StrictMode;
import android.widget.TextView;
import android.widget.Toast;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;

public class MainActivity extends AppCompatActivity {
    private String Device_Name = "temp01"; // 设备名称
    private String Pe_ID = "8VORdlxrZT"; // 产品ID
    private String path_head = "https://iot-api.heclouds.com/thingmodel/query-device-property?product_id=";
    private String path_end = "&device_name=";
    private String path = path_head + Pe_ID + path_end + Device_Name; // 获取属性

    private Handler handler;
    private String temp_value = "N/A", shidu_value = "N/A", light_value = "N/A", pir_value = "N/A", vibration_value = "N/A"; // 初始值
    private TextView wendu_tv, shidu_tv, light_tv, pir_tv, vibration_tv;

    private Runnable task = new Runnable() {
        @Override
        public void run() {
            GetOnenetData(); // 查询云端数据

            // PIR状态判断
            String pirStatus;
            if (pir_value.equals("1")) {
                pirStatus = "活动中";
            } else {
                pirStatus = "休息中";
            }

            // 振动状态判断
            String vibrationStatus;
            if (vibration_value.equals("1")) {
                vibrationStatus = "行为异常";
            } else {
                vibrationStatus = "行为正常";
            }

            // 更新UI组件
            wendu_tv.setText("温度: " + temp_value + " °C");
            shidu_tv.setText("湿度: " + shidu_value + " %");
            light_tv.setText("光照强度: " + light_value);
            pir_tv.setText("PIR状态: " + pirStatus);
            vibration_tv.setText("振动状态: " + vibrationStatus);

            handler.postDelayed(this, 5000); // 每5秒更新一次
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // 允许在主线程中执行网络操作（仅供调试，生产环境应避免）
        if (android.os.Build.VERSION.SDK_INT > 9) {
            StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
            StrictMode.setThreadPolicy(policy);
        }

        // 初始化UI组件
        wendu_tv = findViewById(R.id.textView_temp_value);
        shidu_tv = findViewById(R.id.textView_humidity_value);
        light_tv = findViewById(R.id.textView_light_value);
        pir_tv = findViewById(R.id.textView_pir_value);
        vibration_tv = findViewById(R.id.textView_vibration_value);

        // 初始化Handler并启动任务
        handler = new Handler();
        handler.post(task);
    }

    private void GetOnenetData() {
        HttpURLConnection conn = null;
        try {
            URL url = new URL(path);
            conn = (HttpURLConnection) url.openConnection();
            conn.setConnectTimeout(1500);
            conn.setReadTimeout(1500);
            conn.setRequestMethod("GET");
            conn.setRequestProperty("authorization", "version=2018-10-31&res=products%2F8VORdlxrZT%2Fdevices%2Ftemp01&et=2017881776&method=md5&sign=rvz5GSi9ehQQNAzv%2FTV6cw%3D%3D"); // 替换为实际的Token

            if (conn.getResponseCode() == 200) {
                InputStream is = conn.getInputStream();
                BufferedReader br = new BufferedReader(new InputStreamReader(is));
                StringBuilder response = new StringBuilder();
                String line;
                while ((line = br.readLine()) != null) {
                    response.append(line);
                }
                DealJsonData(response.toString());
            } else {
                Toast.makeText(MainActivity.this, "网络错误，状态码: " + conn.getResponseCode(), Toast.LENGTH_LONG).show();
            }
        } catch (Exception e) {
            e.printStackTrace();
            Toast.makeText(MainActivity.this, "请求失败: " + e.getMessage(), Toast.LENGTH_LONG).show();
        } finally {
            if (conn != null) {
                conn.disconnect();
            }
        }
    }

    private void DealJsonData(String JSON) throws JSONException {
        JSONObject jsonObject = new JSONObject(JSON);
        JSONArray data = jsonObject.optJSONArray("data");
        if (data != null) {
            for (int i = 0; i < data.length(); i++) {
                JSONObject value = data.getJSONObject(i);
                if (value.optString("identifier").equals("CurrentTemperature")) {
                    temp_value = value.optString("value");
                } else if (value.optString("identifier").equals("CurrentHumidity")) {
                    shidu_value = value.optString("value");
                } else if (value.optString("identifier").equals("Light_value")) {
                    light_value = value.optString("value");
                } else if (value.optString("identifier").equals("PIR")) {
                    pir_value = value.optString("value");
                } else if (value.optString("identifier").equals("Vibration")) {
                    vibration_value = value.optString("value");
                }
            }
        }
    }
}