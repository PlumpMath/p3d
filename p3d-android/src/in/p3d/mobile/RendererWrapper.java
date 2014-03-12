package in.p3d.mobile;

import java.nio.ByteBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLSurfaceView.Renderer;

public class RendererWrapper implements Renderer {
	private ByteBuffer modelData = null;
	private String extension = ".bin";
	
	private Object rotateMutex = new Object();
	private float rotateX;
	private float rotateY;
	private float startRotateX;
	private float startRotateY;
	private boolean doRotateCam = false;
	private boolean doStartRotateCam = false;

	public void loadModel(ByteBuffer data) {
		modelData = data;
	}
	
	public void startRotateCam(float x, float y) {
		synchronized (rotateMutex) {
			startRotateX = x;
			startRotateY = y;
			doStartRotateCam = true;
		}
	}

	public void rotateCam(float x, float y) {
		synchronized (rotateMutex) {
			rotateX = x;
			rotateY = y;
			doRotateCam = true;
		}
	}
	
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        P3dViewerJNIWrapper.on_surface_created();
    }
 
    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        P3dViewerJNIWrapper.on_surface_changed(width, height);
    }
 
    @Override
    public void onDrawFrame(GL10 gl) {
    	if(modelData != null) {
    		P3dViewerJNIWrapper.load_model(modelData, modelData.limit(), extension);
    		modelData = null;
    	}
    	synchronized (rotateMutex) {
    		if(doStartRotateCam) {
    			doStartRotateCam = false;
    			P3dViewerJNIWrapper.start_rotate_cam(startRotateX, startRotateY);
    		}
    		if(doRotateCam) {
    			doRotateCam = false;
    	    	P3dViewerJNIWrapper.rotate_cam(rotateX, rotateY);
    		}
		}
    	
        P3dViewerJNIWrapper.on_draw_frame();
    }

}