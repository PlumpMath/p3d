package in.p3d.mobile;

import java.nio.ByteBuffer;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.app.ActivityManager;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ConfigurationInfo;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

public class ViewerActivity extends Activity {
	private static String TAG = "ViewerActivity";
	public static final String ARG_SHORTID = "shortid";
	
	private P3dGLSurfaceView glSurfaceView;
	private boolean rendererSet;
	private ProgressDialog loadingDialog;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
	    super.onCreate(savedInstanceState);
	    
	    ActivityManager activityManager
	        = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
	    ConfigurationInfo configurationInfo = activityManager.getDeviceConfigurationInfo();
	 
	    final boolean supportsEs2 =
	        configurationInfo.reqGlEsVersion >= 0x20000 || isProbablyEmulator();
	 
	    if (supportsEs2) {
	    	P3dViewerJNIWrapper.init_asset_manager(getAssets());
	        glSurfaceView = new P3dGLSurfaceView(this);	 
	        rendererSet = true;
	        setContentView(glSurfaceView);
	        
	        Intent intent = getIntent();
	        String shortid = intent.getStringExtra(ARG_SHORTID);
	        
	        if(shortid == null) {
	        	shortid = "Ui03b"; // horse
//	        	shortid = "TpN5G"; // large monkey
	        }
	        
	        loadModel(shortid);
	    } else {
	        // Should never be seen in production, since the manifest filters
	        // unsupported devices.
	        Toast.makeText(this, "This device does not support OpenGL ES 2.0.",
	                Toast.LENGTH_LONG).show();
	        return;
	    }
    }
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		default:
			return MainActivity.getInstance().handleGlobalMenu(item);
		}
	}

	@Override
	protected void onPause() {
	    super.onPause();
	 
	    if (rendererSet) {
	        glSurfaceView.onPause();
	    }
	}
	 
	@Override
	protected void onResume() {
	    super.onResume();
	 
	    if (rendererSet) {
	        glSurfaceView.onResume();
	    }
	}
	
	@Override
	protected void onDestroy() {
		Log.d(TAG, "destroy");
		super.onDestroy();
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	private boolean isProbablyEmulator() {
	    return Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1
	            && (Build.FINGERPRINT.startsWith("generic")
	                    || Build.FINGERPRINT.startsWith("unknown")
	                    || Build.MODEL.contains("google_sdk")
	                    || Build.MODEL.contains("Emulator")
	                    || Build.MODEL.contains("Android SDK built for x86"));
	}
	
	private void loadModel(String shortid) {
		Log.d(TAG, "Loading: " + shortid);
		loadingDialog = ProgressDialog.show(this, "", getString(R.string.loading), true);
		
		AsyncTask<String, Void, JSONObject> asyncTask = new AsyncTask<String, Void, JSONObject>() {
			@Override
			protected JSONObject doInBackground(String... urls) {
				return Util.getJson(urls[0]);
			}

			@Override
			protected void onPostExecute(JSONObject result) {
				if (result == null) {
					// TODO: error msg
					return;
				}
				try {
					String binUrl = "http://p3d.in"
							+ result.getJSONObject("viewer_model").getString(
									"base_url") + ".r48.bin";
					Log.d(TAG, "Bin url: " + binUrl);
					loadBinary(binUrl);
				} catch (JSONException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		};
		asyncTask.execute("http://p3d.in/api/viewer_models/" + shortid);
	}

	private void loadBinary(String binUrl) {
		Log.d(TAG, "Loading binary: " + binUrl);
		AsyncTask<String, Void, ByteBuffer> asyncTask = new AsyncTask<String, Void, ByteBuffer>() {
			@Override
			protected ByteBuffer doInBackground(String... urls) {
				return Util.getBinary(urls[0]);
			}

			@Override
			protected void onPostExecute(ByteBuffer result) {
				if (result == null) {
					// TODO: error msg
					return;
				}
				try {
					Log.d(TAG, "Got binary: " + result.limit());
					loadingDialog.cancel();
					glSurfaceView.getRenderer().loadModel(result);
				} catch (Exception e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		};
		asyncTask.execute(binUrl);		
	}
}
