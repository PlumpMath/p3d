package in.p3d.gltest;

import java.nio.ByteBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLSurfaceView.Renderer;

public class RendererWrapper implements Renderer {
	private ByteBuffer modelData = null;
	
	public void loadModel(ByteBuffer data) {
		modelData = data;
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
    		P3dViewerJNIWrapper.load_binary(modelData, modelData.limit());
    		modelData = null;
    	}
    	
        P3dViewerJNIWrapper.on_draw_frame();
    }

}