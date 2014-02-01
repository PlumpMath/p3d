package in.p3d.mobile;

import java.io.InputStream;
import java.net.URL;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.AsyncTask;
import android.util.Log;
import android.util.SparseArray;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public class ModelAdapter extends BaseAdapter {

	private static String TAG = "ModelAdapter";

	private Context context;
	private JSONArray models;
	private SparseArray<AsyncTask<String, Void, Bitmap>> asyncTasks = new SparseArray<AsyncTask<String, Void, Bitmap>>();
	private SparseArray<Bitmap> bitmaps = new SparseArray<Bitmap>();

	public void setModels(JSONArray models) {
		this.models = models;
		for (int i = 0; i < asyncTasks.size(); ++i) {
			AsyncTask<String, Void, Bitmap> asyncTask = asyncTasks.valueAt(i);
			if (asyncTask != null) {
				asyncTask.cancel(true);
			}
		}
		asyncTasks.clear();
		bitmaps.clear();
		notifyDataSetChanged();
	}

	public ModelAdapter(Context context, JSONArray models) {
		this.context = context;
		this.models = models;
	}

	@Override
	public int getCount() {
		return models.length();
	}

	@Override
	public Object getItem(int position) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public long getItemId(int position) {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public View getView(final int position, View convertView, ViewGroup parent) {
		LayoutInflater inflater = (LayoutInflater) context
				.getSystemService(Context.LAYOUT_INFLATER_SERVICE);

		View gridView = null;

		JSONObject model;
		try {
			model = models.getJSONObject(position);
			if (convertView == null) {
				// get layout from mobile.xml
				gridView = inflater.inflate(R.layout.model_item, null);
			} else {
				gridView = (View) convertView;
			}

			// set value into textviews
			TextView textView = (TextView) gridView
					.findViewById(R.id.model_item_name);
			textView.setText(model.getString("name"));
			textView = (TextView) gridView
					.findViewById(R.id.model_item_author);
			textView.setText("by " + model.getString("owner"));
			textView = (TextView) gridView
					.findViewById(R.id.model_item_views);
			textView.setText(model.getString("hits"));

			// set image based on selected text
			final ImageView imageView = (ImageView) gridView
					.findViewById(R.id.model_item_img);

			Bitmap bitmap = bitmaps.get(position);
			AsyncTask<String, Void, Bitmap> asyncTask = asyncTasks
					.get(position);
			
			imageView.setImageDrawable(null);

			if(bitmap != null) {
				imageView.setImageBitmap(bitmap);
			} else if (asyncTask == null) {
				asyncTask = new AsyncTask<String, Void, Bitmap>() {
					@Override
					protected Bitmap doInBackground(String... urls) {
						try {
							//Log.d(TAG, String.format("Loading %s", urls[0]));
							Bitmap bitmap = BitmapFactory
									.decodeStream((InputStream) new URL(urls[0])
											.getContent());
							//Log.d(TAG, String.format("Loaded %s", urls[0]));
							return bitmap;
						} catch (Exception e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
							return null;
						}
					}

					@Override
					protected void onPostExecute(Bitmap result) {
						bitmaps.put(position, result);
						if (result != null && imageView.getDrawable() == null) {
							imageView.setImageBitmap(result);
						} else {
							Log.d(TAG, "NOT setting bitmap");
							Log.d(TAG, imageView.getDrawable().toString());
						}
					}
				};
				asyncTask.execute("http://p3d.in"
						+ model.getString("thumbnail"));
				asyncTasks.put(position, asyncTask);
			}

		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return gridView;
	}

}
