package in.p3d.mobile;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;

public class P3dGLSurfaceView extends GLSurfaceView {
	private static String TAG = "P3dGLSurfaceView";
	private GestureDetector detector;
	
	public P3dGLSurfaceView(Context context) {
		super(context);
		detector = new GestureDetector(context, new GestureDetector.SimpleOnGestureListener() {
			@Override
			public boolean onDoubleTap(MotionEvent e) {
				P3dViewerJNIWrapper.reset_cam();
				return false;
			}
		});
	}

	@Override
	public boolean onTouchEvent(MotionEvent e) {
		detector.onTouchEvent(e);
		float x = e.getX() * 2.0f / getWidth() - 1.0f;
	    float y = -e.getY() * 2.0f / getHeight() + 1.0f;
	    switch (e.getAction()) {
	    case MotionEvent.ACTION_DOWN:
	    	//Log.d(TAG, "touch down: " + x + ", " + y);
	    	P3dViewerJNIWrapper.start_rotate_cam(x, y);
		    return true;
        case MotionEvent.ACTION_MOVE:
	    	//Log.d(TAG, "touch move: " + x + ", " + y);
	    	P3dViewerJNIWrapper.rotate_cam(x, y);
        	return true;
	    }
	    
	    return super.onTouchEvent(e);
	}
}
