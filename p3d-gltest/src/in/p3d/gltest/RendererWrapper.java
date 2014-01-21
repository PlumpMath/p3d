package in.p3d.gltest;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLSurfaceView.Renderer;

public class RendererWrapper implements Renderer {
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
        P3dViewerJNIWrapper.on_draw_frame();
    }

}