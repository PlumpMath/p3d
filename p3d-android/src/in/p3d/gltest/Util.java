package in.p3d.gltest;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.ByteBuffer;

import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.DefaultHttpClient;
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
		try {	    	
			HttpClient httpclient = new DefaultHttpClient(); // for port 80 requests!
			HttpGet httpget = new HttpGet(url);
			HttpResponse response = httpclient.execute(httpget);
			HttpEntity entity = response.getEntity();
			is = entity.getContent();
		} catch(Exception e) {
			return null;
		}
	    
		// Read response to string
		try {	    	
			BufferedReader reader = new BufferedReader(new InputStreamReader(is,"utf-8"),8);
			StringBuilder sb = new StringBuilder();
			String line = null;
			while ((line = reader.readLine()) != null) {
				sb.append(line + "\n");
			}
			is.close();
			result = sb.toString();	            
		} catch(Exception e) {
			return null;
		}
 
		// Convert string to object
		try {
			jsonObject = new JSONObject(result);            
		} catch(JSONException e) {
			return null;
		}
    
		return jsonObject;
	}
	
	public static ByteBuffer getBinary(String url) {
		InputStream is = null;
		ByteBuffer data = null;
		
		// HTTP
		HttpResponse response;
		HttpEntity entity;
		try {	    	
			HttpClient httpclient = new DefaultHttpClient(); // for port 80 requests!
			HttpGet httpget = new HttpGet(url);
			response = httpclient.execute(httpget);
			entity = response.getEntity();
			is = entity.getContent();
		} catch(Exception e) {
			Log.e(TAG, "error", e);
			return null;
		}
	    
		// Read response to string
		try {	
			int len = (int) entity.getContentLength();
			Log.d(TAG, "bin len: " + len);
			data = ByteBuffer.allocateDirect(len);
			int len1;
			byte[] temp = new byte[2048];
			while ((len1 = is.read(temp)) > 0) {
				data.put(temp, 0, len1);
			}
			is.close();
		} catch(Exception e) {
			Log.e(TAG, "error", e);
			return null;
		}
   
		return data;
	}
}
