package org.opencv.engine.manager;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.StringTokenizer;

import org.opencv.engine.MarketConnector;
import org.opencv.engine.OpenCVEngineInterface;
import org.opencv.engine.R;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageInfo;
import android.os.Build;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.TextView;
import android.widget.Toast;

public class ManagerActivity extends Activity
{
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        TextView OsVersionView = (TextView)findViewById(R.id.OsVersionValue);
        OsVersionView.setText(Build.VERSION.CODENAME + " (" + Build.VERSION.RELEASE + "), API " + Build.VERSION.SDK_INT);
        
        mInstalledPackageView = (ListView)findViewById(R.id.InstalledPackageList);
        
        mMarket = new MarketConnector(this);
        FillPackageList();
        
        SimpleAdapter adapter = new SimpleAdapter(
        	    this,
        	    mListViewItems,
        	    R.layout.info,
        	    new String[] {"Name", "Version", "Hardware"},
        	    new int[] {R.id.InfoName,R.id.InfoVersion, R.id.InfoHardware}
        	    );
        
        mInstalledPackageView.setAdapter(adapter);
        
        TextView HardwarePlatformView = (TextView)findViewById(R.id.HardwareValue);
        int Platfrom = HardwareDetector.DetectKnownPlatforms();
        int CpuId = HardwareDetector.GetCpuID();
        
        if (HardwareDetector.PLATFORM_UNKNOWN != Platfrom)
        {
        	if (HardwareDetector.PLATFORM_TEGRA == Platfrom)
        	{
        		HardwarePlatformView.setText("Tegra");
        	}
        	else if (HardwareDetector.PLATFORM_TEGRA == Platfrom)
        	{
        		HardwarePlatformView.setText("Tegra 2");
        	}
        	else
        	{
        		HardwarePlatformView.setText("Tegra 3");
        	}
        }
        else
        {
        	if ((CpuId & HardwareDetector.ARCH_X86) == HardwareDetector.ARCH_X86) 
        	{
        		HardwarePlatformView.setText("x86 " + JoinIntelFeatures(CpuId));
        	}
        	else if ((CpuId & HardwareDetector.ARCH_X64) == HardwareDetector.ARCH_X64)
        	{
        		HardwarePlatformView.setText("x64 " + JoinIntelFeatures(CpuId));
        	}
        	else if ((CpuId & HardwareDetector.ARCH_ARMv5) == HardwareDetector.ARCH_ARMv5)
        	{
        		HardwarePlatformView.setText("ARM v5 " + JoinArmFeatures(CpuId));
        	}
        	else if ((CpuId & HardwareDetector.ARCH_ARMv6) == HardwareDetector.ARCH_ARMv6)
        	{
        		HardwarePlatformView.setText("ARM v6 " + JoinArmFeatures(CpuId));
        	}
        	else if ((CpuId & HardwareDetector.ARCH_ARMv7) == HardwareDetector.ARCH_ARMv7)
        	{
        		HardwarePlatformView.setText("ARM v7 " + JoinArmFeatures(CpuId));
        	}
        	else if ((CpuId & HardwareDetector.ARCH_ARMv8) == HardwareDetector.ARCH_ARMv8)
        	{
        		HardwarePlatformView.setText("ARM v8 " + JoinArmFeatures(CpuId));
        	}
        	else
        	{
        		HardwarePlatformView.setText("not detected");
        	}
        }
        
        mUpdateEngineButton = (Button)findViewById(R.id.CheckEngineUpdate);
        mUpdateEngineButton.setOnClickListener(new OnClickListener() {
			
			public void onClick(View v) {
				if (!mMarket.InstallAppFromMarket("org.opencv.engine"))
				{
					Toast toast = Toast.makeText(getApplicationContext(), "Google Play is not avaliable", Toast.LENGTH_SHORT);
					toast.show();
				}
			}
		});
        
        mActionDialog = new AlertDialog.Builder(this).create();
        
        mActionDialog.setTitle("Choose action");
        mActionDialog.setMessage("Choose what you want to do:");
        mActionDialog.setButton("Update", new DialogInterface.OnClickListener() {
			
			public void onClick(DialogInterface dialog, int which) {
				int index = (Integer)mInstalledPackageView.getTag();
				if (!mMarket.InstallAppFromMarket(mInstalledPackageInfo[index].packageName))
				{
					Toast toast = Toast.makeText(getApplicationContext(), "Google Play is not avaliable", Toast.LENGTH_SHORT);
					toast.show();
				}
			}
		});
        
        mActionDialog.setButton3("Remove", new DialogInterface.OnClickListener() {
			
			public void onClick(DialogInterface dialog, int which) {
				int index = (Integer)mInstalledPackageView.getTag();
				if (!mMarket.RemoveAppFromMarket(mInstalledPackageInfo[index].packageName))
				{
					Toast toast = Toast.makeText(getApplicationContext(), "Google Play is not avaliable", Toast.LENGTH_SHORT);
					toast.show();
				}				
			}
		});
        
        mActionDialog.setButton2("Cancel", new DialogInterface.OnClickListener() {
			
			public void onClick(DialogInterface dialog, int which) {
				// nothing
			}
		});
        
        mInstalledPackageView.setOnItemClickListener(new OnItemClickListener() {

			public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long id) {
				
		        mInstalledPackageView.setTag(new Integer((int)id));
		        mActionDialog.show();
			}		
        });
        
        if (!bindService(new Intent("org.opencv.engine.BIND"), mServiceConnection, Context.BIND_AUTO_CREATE))
        {
        	TextView EngineVersionView = (TextView)findViewById(R.id.EngineVersionValue);
        	EngineVersionView.setText("not avaliable");
        }
    }
    
    protected ListView mInstalledPackageView;
    protected Button mUpdateEngineButton;
    protected PackageInfo[] mInstalledPackageInfo;
    protected static final ArrayList<HashMap<String,String>> mListViewItems = new ArrayList<HashMap<String,String>>();
    protected MarketConnector mMarket;
    AlertDialog mActionDialog;
    
    protected ServiceConnection mServiceConnection = new ServiceConnection() {
		
		public void onServiceDisconnected(ComponentName name) {
			// TODO Auto-generated method stub
			
		}
		
		public void onServiceConnected(ComponentName name, IBinder service) {
			TextView EngineVersionView = (TextView)findViewById(R.id.EngineVersionValue);
			OpenCVEngineInterface EngineService = OpenCVEngineInterface.Stub.asInterface(service);
			try {
				EngineVersionView.setText("" + EngineService.getEngineVersion());
			} catch (RemoteException e) {
				EngineVersionView.setText("not avaliable");
				e.printStackTrace();
			}
			unbindService(mServiceConnection);
		}
	};
	
	protected void FillPackageList()
	{
        mInstalledPackageInfo = mMarket.GetInstalledOpenCVPackages();
        mListViewItems.clear();
        
        for (int i = 0; i < mInstalledPackageInfo.length; i++)
        {
        	// Convert to Items for package list view
        	HashMap<String,String> temp = new HashMap<String,String>();
        	temp.put("Name", mMarket.GetApplicationName(mInstalledPackageInfo[i].applicationInfo));

            int idx = 0;
            String OpenCVersion = "unknown";
            String HardwareName = "";
            StringTokenizer tokenizer = new StringTokenizer(mInstalledPackageInfo[i].packageName, "_");
            while (tokenizer.hasMoreTokens())
            {
            	if (idx == 1)
            	{
            		// version of OpenCV
            		OpenCVersion = tokenizer.nextToken();
            	}
            	else if (idx >= 2)
            	{
            		// hardware options
            		HardwareName += tokenizer.nextToken() + " ";
            	}
            	else
            	{
            		tokenizer.nextToken();
            	}
            	idx++;
            }
            
        	temp.put("Version", NormalizeVersion(OpenCVersion, mInstalledPackageInfo[i].versionName));
        	temp.put("Hardware", HardwareName);
        	mListViewItems.add(temp);
        }
	}
    
	protected String NormalizeVersion(String OpenCVersion, String PackageVersion)
	{
		int dot = PackageVersion.indexOf(".");
		return OpenCVersion.substring(0,  OpenCVersion.length()-1) + "." + 
			OpenCVersion.toCharArray()[OpenCVersion.length()-1] + "." + 
			PackageVersion.substring(0, dot) + " rev " + PackageVersion.substring(dot+1);		
	}
	
    protected String ConvertPackageName(String Name, String Version)
    {
    	return Name + " rev " + Version;
    }
    
    protected String JoinIntelFeatures(int features)
    {
    	// TODO: update if package will be published
    	return "";
    }
    
    protected String JoinArmFeatures(int features)
    {
    	// TODO: update if package will be published
    	if ((features & HardwareDetector.FEATURES_HAS_NEON) == HardwareDetector.FEATURES_HAS_NEON)
    	{
    		return "with Neon";
    	}
    	else if ((features & HardwareDetector.FEATURES_HAS_VFPv3) == HardwareDetector.FEATURES_HAS_VFPv3)
    	{
    		return "with VFP v3";
    	}
    	else if ((features & HardwareDetector.FEATURES_HAS_VFPv3d16) == HardwareDetector.FEATURES_HAS_VFPv3d16)
    	{
    		return "with VFP v3d16";
    	}
    	else
    	{
    		return "";
    	}
    }
}
