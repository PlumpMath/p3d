package in.p3d.mobile;

import java.io.File;
import java.io.IOException;
import java.util.Locale;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.app.ActionBar;
import android.app.FragmentTransaction;
import android.content.Intent;
import android.net.Uri;
import android.net.http.HttpResponseCache;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.GridView;
import android.widget.TextView;

public class MainActivity extends FragmentActivity implements
		ActionBar.TabListener {

	private static String TAG = "MainActivity";
	private static MainActivity singleton;

	/**
	 * The {@link android.support.v4.view.PagerAdapter} that will provide
	 * fragments for each of the sections. We use a
	 * {@link android.support.v4.app.FragmentPagerAdapter} derivative, which
	 * will keep every loaded fragment in memory. If this becomes too memory
	 * intensive, it may be best to switch to a
	 * {@link android.support.v4.app.FragmentStatePagerAdapter}.
	 */
	SectionsPagerAdapter mSectionsPagerAdapter;

	/**
	 * The {@link ViewPager} that will host the section contents.
	 */
	ViewPager mViewPager;

	public static MainActivity getInstance() {
		return singleton;
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		singleton = this;
		setContentView(R.layout.activity_main);

		// Http cache
		try {
			File httpCacheDir = new File(getApplicationContext().getCacheDir(),
					"http");
			long httpCacheSize = 10 * 1024 * 1024; // 10 MiB
			HttpResponseCache.install(httpCacheDir, httpCacheSize);
		} catch (IOException e) {
			Log.i(TAG, "HTTP response cache installation failed:" + e);
		}
		
		// Set up the action bar.
		final ActionBar actionBar = getActionBar();
		actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);

		// Create the adapter that will return a fragment for each of the three
		// primary sections of the app.
		mSectionsPagerAdapter = new SectionsPagerAdapter(
				getSupportFragmentManager());

		// Set up the ViewPager with the sections adapter.
		mViewPager = (ViewPager) findViewById(R.id.pager);
		mViewPager.setAdapter(mSectionsPagerAdapter);

		// When swiping between different sections, select the corresponding
		// tab. We can also use ActionBar.Tab#select() to do this if we have
		// a reference to the Tab.
		mViewPager
				.setOnPageChangeListener(new ViewPager.SimpleOnPageChangeListener() {
					@Override
					public void onPageSelected(int position) {
						actionBar.setSelectedNavigationItem(position);
					}
				});

		// For each of the sections in the app, add a tab to the action bar.
		for (int i = 0; i < mSectionsPagerAdapter.getCount(); i++) {
			// Create a tab with text corresponding to the page title defined by
			// the adapter. Also specify this Activity object, which implements
			// the TabListener interface, as the callback (listener) for when
			// this tab is selected.
			actionBar.addTab(actionBar.newTab()
					.setText(mSectionsPagerAdapter.getPageTitle(i))
					.setTabListener(this));
		}
		
		// try force loading libs
		P3dViewerJNIWrapper.init();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	protected void onStop() {
		HttpResponseCache cache = HttpResponseCache.getInstalled();
		if (cache != null) {
			cache.flush();
		}

		super.onStop();
	}
	
	@Override
	public void onTabSelected(ActionBar.Tab tab,
			FragmentTransaction fragmentTransaction) {
		// When the given tab is selected, switch to the corresponding page in
		// the ViewPager.
		mViewPager.setCurrentItem(tab.getPosition());
	}

	@Override
	public void onTabUnselected(ActionBar.Tab tab,
			FragmentTransaction fragmentTransaction) {
	}

	@Override
	public void onTabReselected(ActionBar.Tab tab,
			FragmentTransaction fragmentTransaction) {
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case R.id.action_exit:
			exitApp();
			return true;
		case R.id.action_update:
			Intent intent = new Intent(Intent.ACTION_VIEW,
					Uri.parse("http://p3d.in/"));
			startActivity(intent);
			return true;
		default:
			return super.onOptionsItemSelected(item);
		}
	}
	
	public void exitApp() {
		finish();
        System.exit(0);
	}
	
	/**
	 * A {@link FragmentPagerAdapter} that returns a fragment corresponding to
	 * one of the sections/tabs/pages.
	 */
	public class SectionsPagerAdapter extends FragmentPagerAdapter {

		public SectionsPagerAdapter(FragmentManager fm) {
			super(fm);
		}

		@Override
		public Fragment getItem(int position) {
			// getItem is called to instantiate the fragment for the given page.
			// Return a DummySectionFragment (defined as a static inner class
			// below) with the page number as its lone argument.
			Fragment fragment = new DummySectionFragment();
			Bundle args = new Bundle();
			switch (position) {
			case 0:
				args.putString(DummySectionFragment.ARG_SECTION_URL,
						"http://p3d.in/api/p3d_models?staffPicks=1&allUsers=1&offset=0");
				break;
			case 1:
				args.putString(DummySectionFragment.ARG_SECTION_URL,
						"http://p3d.in/api/p3d_models?popular=1&allUsers=1&offset=0");
				break;
			case 2:
				args.putString(DummySectionFragment.ARG_SECTION_URL,
						"http://p3d.in/api/p3d_models?sort=-added&allUsers=1&offset=0");
				break;
			}

			
			fragment.setArguments(args);
			return fragment;
		}

		@Override
		public int getCount() {
			// Show 3 total pages.
			return 3;
		}

		@Override
		public CharSequence getPageTitle(int position) {
			Locale l = Locale.getDefault();
			switch (position) {
			case 0:
				return getString(R.string.title_section1).toUpperCase(l);
			case 1:
				return getString(R.string.title_section2).toUpperCase(l);
			case 2:
				return getString(R.string.title_section3).toUpperCase(l);
			}
			return null;
		}
	}

	/**
	 * A dummy fragment representing a section of the app, but that simply
	 * displays dummy text.
	 */
	public static class DummySectionFragment extends Fragment {
		/**
		 * The fragment argument representing the section number for this
		 * fragment.
		 */
		public static final String ARG_SECTION_URL = "section_url";
		JSONArray models = null;
		AsyncTask<String, Void, JSONObject> asyncTask = null;
		ModelAdapter adapter = null;

		public DummySectionFragment() {
		}

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container,
				Bundle savedInstanceState) {

			if(adapter == null) {
				adapter = new ModelAdapter(getActivity(), new JSONArray());
			}
			
			View rootView = inflater.inflate(R.layout.fragment_main_dummy,
					container, false);
			final TextView dummyTextView = (TextView) rootView
					.findViewById(R.id.section_label);

			dummyTextView.setVisibility(View.VISIBLE);

			GridView gridView = (GridView) rootView
					.findViewById(R.id.model_grid);
			gridView.setAdapter(adapter);
			gridView.setColumnWidth(getResources().getDimensionPixelSize(R.dimen.model_item_width) + 20);

			if (asyncTask == null) {
				asyncTask = new AsyncTask<String, Void, JSONObject>() {
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
							models = result.getJSONArray("p3d_models");
							Log.d(TAG, String.format("Got %d models", models.length()));
							dummyTextView.setVisibility(View.GONE);
							adapter.setModels(models);
						} catch (JSONException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
					}
				};
				asyncTask.execute(getArguments().getString(ARG_SECTION_URL));
			} else if (models != null) {
				dummyTextView.setVisibility(View.GONE);
			}
			
			gridView.setOnItemClickListener(new OnItemClickListener() {

				@Override
				public void onItemClick(AdapterView<?> parent, View v,
						int position, long id) {
					Intent intent;
					try {
//						intent = new Intent(Intent.ACTION_VIEW, Uri
//								.parse("http://p3d.in/"
//										+ models.getJSONObject(position).getString(
//												"shortid")));
						intent = new Intent(getActivity(), ViewerActivity.class);
						intent.putExtra(ViewerActivity.ARG_SHORTID,
								models.getJSONObject(position).getString("shortid"));
						startActivity(intent);
					} catch (Exception e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			});
			
			return rootView;
		}
	}

}
