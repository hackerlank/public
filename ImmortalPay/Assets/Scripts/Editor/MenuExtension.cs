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

	[MenuItem("Extension/Update Version",false,122)]
	static void UpdateVersion(){
		var path="Assets/Resources/"+Config.file+".txt";
		string buf = System.IO.File.ReadAllText(path);

		//store comments
		var comments="";
		string[] lines = buf.Split('\n');
		foreach(var line in lines){
			if(line.StartsWith("#") || line.StartsWith("//"))
				comments+=line+"\n";
		}

		//parse
		var dict=Utils.ParseIni(buf);
		dict["user"]	=System.Environment.UserName;
		dict["version"]	=UnityEditor.PlayerSettings.shortBundleVersion;
		dict["date"]	=System.DateTime.Now.ToString("yyyy-MM-dd HH:mm");
		dict["build"]	=UnityEditor.PlayerSettings.
		#if UNITY_IPHONE
			bundleVersion;
		#else
			Android.bundleVersionCode.ToString();
		#endif

		buf=comments;
		foreach(var kv in dict)
			buf+=kv.Key+"="+kv.Value+"\n";
		System.IO.File.WriteAllText (path,buf);
	}
}
