package in.p3d.mobile;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.ByteBuffer;
import java.util.ArrayList;

import org.json.JSONException;
import org.json.JSONObject;

import android.util.Log;

public class Util {
	static private String TAG = "Util";
	
	public static JSONObject getJson(String url) {
		InputStream is = null;
		String result = "";
		JSONObject jsonObject = null;
		
		// HTTP
		HttpURLConnection urlConnection;
		try {
			urlConnection = (HttpURLConnection) new URL(url).openConnection();
			try {
				is = new BufferedInputStream(urlConnection.getInputStream());

				// Read response to string
				BufferedReader reader = new BufferedReader(
						new InputStreamReader(is, "utf-8"), 8);
				StringBuilder sb = new StringBuilder();
				String line = null;
				while ((line = reader.readLine()) != null) {
					sb.append(line + "\n");
				}
				is.close();
				result = sb.toString();
			} catch (Exception e) {
				Log.e(TAG, "error", e);
				return null;
			} finally {
				urlConnection.disconnect();
			}
		} catch (Exception e) {
			Log.e(TAG, "error", e);
			return null;
		}
 
		// Convert string to object
		try {
			jsonObject = new JSONObject(result);            
		} catch(JSONException e) {
			Log.e(TAG, "error", e);
			return null;
		}
    
		return jsonObject;
	}
	
	public static ByteBuffer getBinary(String url) {
		InputStream is = null;
		ByteBuffer data = null;
		
		// HTTP
		HttpURLConnection urlConnection;
		try {
			urlConnection = (HttpURLConnection) new URL(url).openConnection();
			try {
				is = new BufferedInputStream(urlConnection.getInputStream());
				
				int totalLen = 0;
				int len;
				ArrayList<byte[]> chunks = new ArrayList<byte[]>();
				while(true) {
					byte[] temp = new byte[16384];
					len = is.read(temp);
					if(len <= 0) {
						break;
					}
					totalLen += len;
					chunks.add(temp);
				}
				
				data = ByteBuffer.allocateDirect(totalLen);
				for(byte[] temp: chunks) {
					data.put(temp, 0, Math.min(totalLen, temp.length));
					totalLen -= temp.length;
				}
				
				is.close();
			} catch(Exception e) {
				Log.e(TAG, "error", e);
				return null;
			} finally {
				urlConnection.disconnect();
			}
		} catch (Exception e) {
			Log.e(TAG, "error", e);
			return null;
		}

   
		return data;
	}
}
