using UnityEngine;
using UnityEditor;
using System.Collections;
using System.IO;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using UnityEngine.UI;

public class MenuExtension {

	[MenuItem("Extension/ClearCache")]
	static void CleanCache()
	{
		Caching.CleanCache();
	}
	
	[MenuItem("Extension/ClearPlayerPrefs")]
	static void ClearPlayerPrefs()
	{
		PlayerPrefs.DeleteAll();
	}

	[MenuItem("Extension/Reimport UI Assemblies", false, 80)]
	static void ReimportUI() {
		#if UNITY_4_7
		var path = EditorApplication.applicationContentsPath + "/UnityExtensions/Unity/GUISystem/{0}/{1}";
		var version = Regex.Match(Application.unityVersion, @"^[0-9]+\.[0-9]+\.[0-9]+").Value;
		#else
		var path = EditorApplication.applicationContentsPath + "/UnityExtensions/Unity/GUISystem/{1}";
		var version = string.Empty;
		#endif
		string engineDll = string.Format(path, version, "UnityEngine.UI.dll");
		string editorDll = string.Format(path, version, "Editor/UnityEditor.UI.dll");
		ReimportDll(engineDll);
		ReimportDll(editorDll);
		
	}
	static void ReimportDll(string path) {
		if (File.Exists(path))
			AssetDatabase.ImportAsset(path, ImportAssetOptions.ForceUpdate | ImportAssetOptions.DontDownloadFromCacheServer);
		else
			Debug.LogError(string.Format("DLL not found {0}", path));
	}

	[MenuItem("Extension/Create AssetBundle",false,121)]
	static void CreateAssetBundle(){
		EditorCoroutine.start (ReCreateAssetBundlesCo());
	}
	static bool filtered(string file){
		return file.EndsWith(".meta")||file.EndsWith(".cs")
			||file.Contains("/.")||file.Contains("DS_Store");
	}
	static IEnumerator ReCreateAssetBundlesCo(int priority = 1,bool force=false){
		/*
		List<string> list=new List<string>();
		foreach(BundleData bd in BMDataAccessor.Bundles)list.Add(bd.name);
		foreach(string name in list)
			BundleManager.RemoveBundle(name);
		*/
		EditorUtility.DisplayDialog ("Assetbundles", "Assetbundles will be re-created now. You will get pop-up when it's finished.", "Ok");
		BundleData[] bundlesToDelete = BundleManager.Roots.ToArray ();
		string[] files = Directory.GetFiles ("Assets/ResourcesForStreaming/","*.*",SearchOption.AllDirectories);
		foreach (string file in files) {
			if(filtered(file))continue;
			string name = file.Replace("Assets/ResourcesForStreaming/","");
			string[] nameParts = name.Split('.');
			name = nameParts[0];
			bool found = false;
			for(int i=0;i<bundlesToDelete.Length;i++){
				if((bundlesToDelete[i]!=null) && (bundlesToDelete[i].name == name)){
					found = true;
					bundlesToDelete[i] = null;
					break;
				}
			}
			bool succ = found || BundleManager.CreateNewBundle(name,string.Empty,false);
			if(succ&&(!found)){
				BundleManager.AddPathToBundle(file,name);
				BundleManager.GetBundleData(name).priority = priority;
			}
			yield return null;
		}
		foreach (BundleData bd in bundlesToDelete)
			if (bd != null)
				BundleManager.RemoveBundle (bd.name);
		if(force){
			foreach (BundleData bd in BundleManager.Roots)bd.priority = priority;
			BMDataAccessor.SaveBundleData();
		}
		EditorUtility.DisplayDialog ("Assetbundles", "Assetbundles re-created", "Ok");
	}

	[MenuItem("Extension/Update Version",false,122)]
	static void UpdateVersion(){
		var path="Assets/Resources/"+Configs.file+".txt";
		string buf = System.IO.File.ReadAllText(path);
		var dict=Utils.ParseIni(buf);
		dict["user"]	=System.Environment.UserName;
		dict["build"]	=UnityEditor.PlayerSettings.shortBundleVersion;
		dict["date"]	=System.DateTime.Now.ToString("yyyy-MM-dd HH:mm");
		#if UNITY_IPHONE
		dict["version"]	=UnityEditor.PlayerSettings.bundleVersion;
		#else
		dict["version"]	=UnityEditor.PlayerSettings.Android.bundleVersionCode.ToString();
		#endif

		buf="";
		foreach(var kv in dict)
			buf+=kv.Key+"="+kv.Value+"\n";
		System.IO.File.WriteAllText (path,buf);
	}
}
