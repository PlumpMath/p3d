package in.p3d.gltest;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

public class MainActivity extends Activity {
	private GLSurfaceView glSurfaceView;
	private boolean rendererSet;
	
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
	        glSurfaceView = new GLSurfaceView(this);
	 
	        if (isProbablyEmulator()) {
	            // Avoids crashes on startup with some emulator images.
	            glSurfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
	        }
	 
	        glSurfaceView.setEGLContextClientVersion(2);
	        glSurfaceView.setRenderer(new RendererWrapper());
	        rendererSet = true;
	        setContentView(glSurfaceView);
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
		case R.id.action_exit:
			finish();
            System.exit(0);
			return true;
		default:
			return super.onOptionsItemSelected(item);
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
}
