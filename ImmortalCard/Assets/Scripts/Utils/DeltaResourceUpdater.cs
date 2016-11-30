using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
#if UNITY_EDITOR && !UNITY_WEBPLAYER
using UnityEditor;
#endif
/*
public class Resource {

	public enum State
	{
		MISSING = 1,
		CACHING = 2,
		CACHED = 3,
		EXISTS_IN_RESOURCES = 4,
		EXISTS_ONLY_IN_RESOURCES = 5,
	}
	private string VersionKey{get{return "BUNDLEVERSION_"+definition.name;}}
	private string UVersionKey{get{return "BUNDLEUNITYVERSION_"+definition.name;}}
	private int UVersion = 0;
	public State state;
	public ResourceDefinition definition;
	public bool IsRequired{get{return !((definition.type==ResourceType.OPTIONAL)||(definition.type==ResourceType.OPTIONAL_OVERRIDING));}}
	public WWW www = null;

	public Resource(ResourceDefinition definition)
	{
		this.definition = definition;
		switch (definition.type) {
		case ResourceType.BUILTINCLIENT:
			state = State.EXISTS_ONLY_IN_RESOURCES;
			break;
		case ResourceType.OPTIONAL_OVERRIDING:
			state = State.EXISTS_IN_RESOURCES;
			break;
		case ResourceType.MEMORYONLY:
		case ResourceType.OPTIONAL:
		case ResourceType.REQUIRED:
		case ResourceType.OVERRIDING:
			state = State.CACHED;
			break;
		}
		UVersion = DeltaPrefs.GetInt (UVersionKey, 0);
		if ((definition.type != ResourceType.MEMORYONLY) && (definition.type != ResourceType.BUILTINCLIENT)) {
			string currentVersion = DeltaPrefs.GetString (VersionKey, "");
			if (currentVersion == definition.version)
					state = State.CACHED;
			else
					UVersion++;
		}
	}

	private bool isCaching = false;
	public IEnumerator Cache()
	{
		if (isCaching)
			yield break;
		if (state == State.EXISTS_ONLY_IN_RESOURCES)
			yield break;
		if (state == State.CACHED)
			yield break;
		if (definition.type == ResourceType.MEMORYONLY)
			yield break;
		if (definition.type == ResourceType.BUILTINCLIENT) 
			yield break;
		isCaching = true;
		state = State.CACHING;
		do
		{
			www = WWW.LoadFromCacheOrDownload ("HERE SHOULD BE PATH TO SERVER, PLATFORM AND UNITY VERSION"+definition.path,UVersion);
			www.threadPriority = ThreadPriority.Low;
			yield return www;
		}while (www.error!=null);
		www.Dispose ();
		www = null;
#if CORRUPT_PREFS
		DeltaPrefs.SetString (VersionKey, "warnuts"+definition.version);
		DeltaPrefs.SetString(UVersionKey, "warnuts"+UVersion);
#else
		DeltaPrefs.SetString (VersionKey, definition.version);
		DeltaPrefs.SetInt (UVersionKey, UVersion);
#endif
		DeltaPrefs.Save ();
		state = State.CACHED;
		isCaching = false;
	}

	private bool isSpawining = false;
	private Object obj = null;
	object objBuildIn=null;
	public IEnumerator Spawn(){
#if DEVELOPMENT_BUILD
		float logtime = Time.realtimeSinceStartup;
#endif
		if (isSpawining || (obj != null)) {
			yield return CDbg.Null ();
		}
		if(Configs.Testing.skipResourceUpdated>0)
			definition.type = ResourceType.BUILTINCLIENT;
		else
			if (IsRequired && (state < State.CACHED))yield break;
		if (definition.type == ResourceType.BUILTINCLIENT) {
			foreach(object o in Game.ToEnumerable(SpawnFromResourcesFolder(definition.name))){
				if((o is Object)&&(o!=null)){
					obj = (Object)o;
				}else yield return o;
			}
#if DEVELOPMENT_BUILD
			//Debug.Log("Resource "+definition.name+" loading time: "+(Time.realtimeSinceStartup-logtime));
#endif
			yield break;
		}
		if ((definition.type == ResourceType.OPTIONAL_OVERRIDING) && (state != State.CACHED)) {
			foreach(object o in Game.ToEnumerable(SpawnFromResourcesFolder(definition.name))){
				if((o is Object)&&(o!=null)){
					obj = (Object)o;
				}else yield return o;
			}
#if DEVELOPMENT_BUILD
			//Debug.Log("Resource "+definition.name+" loading time: "+(Time.realtimeSinceStartup-logtime));
#endif
			yield break;
		}
		if ((definition.type == ResourceType.OPTIONAL) && (state != State.CACHED)) {
			foreach(object o in Game.ToEnumerable(SpawnFromResourcesFolder(definition.fallbackName))){
				if((o is Object)&&(o!=null)){
					obj = (Object)o;
				}else yield return o;
			}
#if DEVELOPMENT_BUILD
			//Debug.Log("Resource "+definition.name+" loading time: "+(Time.realtimeSinceStartup-logtime));
#endif
			yield break;
		}
		isSpawining = true;
		string url = DeltaResourceUpdater.MakeUrl(definition.name);
		while(www==null){
			yield return Game.StartCo(DownloadManager.Instance.WaitDownload(url,10));
			www = DownloadManager.Instance.GetWWW(url);
		}
		if(www != null){
			var name=Path.GetFileNameWithoutExtension(definition.name);
			Object o=null;
			if(definition.name.Contains("Sprite/")){
				//sprites
				o=www.assetBundle.Load(name,typeof(Sprite));
				if(o!=null)obj = Instantiate(o,name);
			}else if(definition.name.Contains("content/")){
				//contents
				var asset1=www.assetBundle.Load(name,typeof(TextAsset));
				//var asset=www.assetBundle.Load(name) as TextAsset;
				var asset=Instantiate(asset1,name) as TextAsset;
				byte[] bytes = null;
				bool _b64_encode=true;
				var rep_t=typeof(World);
				if(name=="world")rep_t=typeof(World);
				else if(name=="creatures")rep_t=typeof(RetrieveCreatureTypeRep);
				try {
					if (_b64_encode)bytes = System.Convert.FromBase64String(asset.text);//from base64
					else bytes = asset.bytes;//raw bytes
					MemoryStream ms = new MemoryStream (bytes);//from bytes
					if(rep_t!=null)
						objBuildIn =Game.instance.resourceUpdater.serializer.Deserialize (ms, null, rep_t);//from stream
				} catch (System.Exception e) {
					Debug.LogError (e);
					if(rep_t!=null)
						Ndbg.Log ("Problem when receiving " + rep_t + " message\n" +
						          "\tEndOfStreamException\n" +
						          "\tWWW.text.length: " + www.text.Length + "\n" +
						          "\tWWW.text: " + www.text + "\n" +
						          "\tbytes.length: " + bytes.Length);
				}
			}else{
				//gameobjects
				o=www.assetBundle.Load(name,typeof(GameObject));
				if(o!=null)obj = Instantiate(o,name);
			}
			www.assetBundle.Unload (false);
			DownloadManager.Instance.DisposeWWW(url);
			//www.Dispose ();
			www = null;
		}
		isSpawining = false;
#if DEVELOPMENT_BUILD
		//Debug.Log("Resource "+definition.name+" loading time: "+(Time.realtimeSinceStartup-logtime));
#endif
	}

	public IEnumerator load<T>(System.Action<Object> handler){
		if (isSpawining)yield return CDbg.Null();
#if DEVELOPMENT_BUILD
		float logtime = Time.realtimeSinceStartup;
#endif
		Object lo=null;
		if(Configs.Testing.skipResourceUpdated>0)
			definition.type = ResourceType.BUILTINCLIENT;
		if (definition.type == ResourceType.BUILTINCLIENT) {
			foreach(object o in Game.ToEnumerable(SpawnFromResourcesFolder(definition.name))){
				if((o is Object)&&(o!=null)){
					lo = (Object)o;
				}else yield return o;
			}
		}else if ((definition.type == ResourceType.OPTIONAL_OVERRIDING) && (state != State.CACHED)) {
			foreach(object o in Game.ToEnumerable(SpawnFromResourcesFolder(definition.name))){
				if((o is Object)&&(o!=null)){
					lo = (Object)o;
				}else yield return o;
			}
		}else if ((definition.type == ResourceType.OPTIONAL) && (state != State.CACHED)) {
			foreach(object o in Game.ToEnumerable(SpawnFromResourcesFolder(definition.fallbackName))){
				if((o is Object)&&(o!=null)){
					lo = (Object)o;
				}else yield return o;
			}
		}else{
			isSpawining = true;
			string url = DeltaResourceUpdater.MakeUrl(definition.name);
			while(www==null){
				yield return Game.StartCo(DownloadManager.Instance.WaitDownload(url,10));
				www = DownloadManager.Instance.GetWWW(url);
			}
			if(www != null){
				var name=Path.GetFileNameWithoutExtension(definition.name);
				lo=www.assetBundle.Load(name,typeof(T));
				if(lo!=null)lo = Instantiate(lo,name);
				www.assetBundle.Unload (false);
				DownloadManager.Instance.DisposeWWW(url);	//www.Dispose ();
				www = null;
			}
			isSpawining = false;
		}
#if DEVELOPMENT_BUILD
		//Debug.Log("Resource "+definition.name+" loading time: "+(Time.realtimeSinceStartup-logtime));
#endif
		if(lo!=null)handler.Invoke(lo);
	}

	public bool cache(int priority){
		if (!isSpawining){
			//#pragma warning disable 0219
			//Object o=null;
			//#pragma warning restore 0219
			if(Configs.Testing.skipResourceUpdated>0)
				definition.type = ResourceType.BUILTINCLIENT;
			if (definition.type == ResourceType.BUILTINCLIENT) {
				//o = SpawnFromResourcesFolder(definition.name);
				Game.StartCo(SpawnFromResourcesFolder(definition.name));
			}else if ((definition.type == ResourceType.OPTIONAL_OVERRIDING) && (state != State.CACHED)) {
				//o = SpawnFromResourcesFolder(definition.name);
				Game.StartCo(SpawnFromResourcesFolder(definition.name));
			}else if ((definition.type == ResourceType.OPTIONAL) && (state != State.CACHED)) {
				//o = SpawnFromResourcesFolder(definition.fallbackName);
				Game.StartCo(SpawnFromResourcesFolder(definition.fallbackName));
			}else{
				string url = DeltaResourceUpdater.MakeUrl(definition.name);
				DownloadManager.Instance.StartDownload(url,priority);
				return false;
			}
		}
		return true;
	}

	private IEnumerator<Object> SpawnFromResourcesFolder(string name)
	{
		if (Game.instance.resourceUpdater.SyncLoading) {
			foreach(Object o in Game.ToEnumerable(SpawnFromResourcesFolderSync(name)))yield return o;
			yield break;
		}

		if(name.Contains("Sprite/")){
			ResourceRequest req = Resources.LoadAsync(name,typeof(Sprite));
			while(!req.isDone)yield return null;
			yield return Instantiate (req.asset as Sprite,name);
			yield break;
		}
		ResourceRequest req2 = Resources.LoadAsync(name);
		while(!req2.isDone)yield return null;
		yield return Instantiate (req2.asset,name);
	}
	
	private IEnumerator<Object> SpawnFromResourcesFolderSync(string name)
	{
		if(name.Contains("Sprite/")){
			yield return Instantiate (Resources.Load(name,typeof(Sprite)),name);
			yield break;
		}
		yield return Instantiate (Resources.Load(name),name);
	}

	Object Instantiate(Object obj,string objname){
		if(obj==null){
			Debug.LogError("SpawnFromResourcesFolder error: "+objname);
			return null;
		}else
			return ((obj is Sprite)||(obj is Texture2D))?obj:MonoBehaviour.Instantiate(obj);
	}

	public Object RetrieveAndNullify()
	{
		Object tmp = obj;
		obj = null;
		return tmp;
	}

	public object RetrieveAndNullifyBuildIn()
	{
		var tmp = objBuildIn;
		objBuildIn = null;
		return tmp;
	}
}


public class DeltaResourceUpdater : MonoBehaviour {

	public List<Resource> Required = new List<Resource>();
	public List<Resource> Optional = new List<Resource>();
	public Dictionary<string,Resource> All;
	private bool isResourceUpdatingReq = false;
	private bool isResourceUpdatingOpt = false;
	public bool SyncLoading = true;

	public ProtocolsSerializer serializer=new ProtocolsSerializer();

	void Awake () {
		InitBuiltInResourceDefinitions ();
		SetupProgressBar (0f);
	}

	void Start () {
	}

	void Update () {
		if (isResourceUpdatingReq)SetProgressBar (RequiredDownloadingProgress());
		else if (isResourceUpdatingOpt)SetProgressBar (OptionalDownloadingProgress());
	}

	public void SetProgressBar(float progress){
		if (isResourceUpdatingReq || isResourceUpdatingOpt)
						return;
		SetupProgressBar (progress);
	}

	private void SetupProgressBar(float progress){
		if(null==CloudPanel.Instance)return;
		Game.instance.Actived = (progress!=0&&progress!=1);
	}

	public IEnumerator BindProgressBar(WWW www)
	{
		while (!www.isDone) {
			SetProgressBar (0.5f);
			yield return CDbg.Null();
		}
		SetProgressBar (0f);
	}

	public void InitServerResources(){
		Game.instance.send<ResourceUpdaterRequest,ResourceUpdaterList> (new ResourceUpdaterRequest (), this,Constants.SERVER.ASSETS);
	}

	private void OnResourceUpdaterList(ResourceUpdaterList list){
		foreach (ResourceDefinition rd in list.list)
						AddResource (rd);
	}

	public void InitBuiltInResourceDefinitions(){
		System.Type t = typeof(ResourceDefinitionList);
		MethodInfo method = t.GetMethod("GenerateBuiltinResourceList", BindingFlags.Static | BindingFlags.Public);
		Dictionary<string,ResourceDefinition> defs = (Dictionary<string,ResourceDefinition>)method.Invoke(null, null);
		All = new Dictionary<string, Resource> ();
		foreach (ResourceDefinition rd in defs.Values) {
			All.Add(rd.name,new Resource(rd));
		}
	}

	public void AddResource(ResourceDefinition resource){
		AddResource (new Resource (resource));
	}

	public List<ResourceDefinition> serverResources = null;
	public void AddResources(List<ResourceDefinition> resources){
		serverResources = resources;
		foreach (ResourceDefinition rd in resources) {
			Ndbg.Log("Additional resource definition file: "+rd.name+":"+rd.path);
			AddResource (rd);
		}
	}

	public void AddServerResource(string name, string path){
		ResourceDefinition resource = new ResourceDefinition ();
		resource.name = name;
		resource.path = path;
		resource.type = ResourceType.REQUIRED;
		AddResource (resource);
	}

	void AddResource(Resource resource){
		if (All.ContainsKey (resource.definition.name)) {
			All [resource.definition.name] = resource;
		} else
			All.Add (resource.definition.name, resource);
		if (resource.IsRequired)
			Required.Add (resource);
		else
			Optional.Add (resource);
	}

	public void AddResource(string name,WWW www){
		var rd=new ResourceDefinition();
		rd.name=name;
		rd.type=ResourceType.OVERRIDING;
		var res=new Resource(rd);
		res.www=www;
		AddResource(res);
	}

	public float RequiredDownloadingProgress(){
		float total = 0;
		float finished = 0;
		foreach (Resource r in Required) {
			total+=1f;
			if(r.state >= Resource.State.CACHED) finished+=1f;
			else if((r.www!=null)&&(r.www.error!=null))finished += r.www.progress;
		}
		if (total == 0)
			return 1f;
		return finished / total;
	}

	public float OptionalDownloadingProgress(){
		float total = 0;
		float finished = 0;
		foreach (Resource r in Optional) {
			total+=1f;
			if(r.state >= Resource.State.CACHED) finished+=1f;
			else if((r.www!=null)&&(r.www.error!=null))finished += r.www.progress;
		}
		if (total == 0)
			return 1f;
		return finished / total;
	}

	public IEnumerator CacheRequired()
	{
		isResourceUpdatingReq = true;
		foreach (Resource r in Required)
			yield return StartCoroutine(r.Cache ());
		isResourceUpdatingReq = false;
	}

	public IEnumerator CacheOptional()
	{
		isResourceUpdatingOpt = true;
		foreach (Resource r in Optional)
			yield return StartCoroutine(r.Cache ());
		isResourceUpdatingOpt = false;
	}

	public void load(string url,System.Action<Object,Hashtable> handler=null,Hashtable param=null,System.Action failHandler=null){
		Game.StartCo(_asyncLoad(url,handler,param));
	}

	IEnumerator _asyncLoad(string url,System.Action<Object,Hashtable> handler,Hashtable param,System.Action failHandler=null){
		Resource res=null;
		try{
			res=Game.instance.resourceUpdater.All[url];
		}catch{
			Debug.LogError("Resource "+url+" not found.");
		}
		Object go=null;
		if(null!=res){
			IEnumerator loading = res.Spawn();
			var t=Time.realtimeSinceStartup;
			while (loading.MoveNext()&&Time.realtimeSinceStartup-t<Configs.Testing.NetworkTimeout)yield return CDbg.Null();
			go = Game.instance.resourceUpdater.All [url].RetrieveAndNullify ();
			if(go==null&&Configs.Tutorial.MissingResourcesLogging>0)StatsHelper.Log(StatsType.ASSETS_ERROR,new System.Collections.Generic.Dictionary<string, object>(){{Time.realtimeSinceStartup-t<Configs.Testing.NetworkTimeout?"load error":"load timeout",url},{"stack",UnityEngine.StackTraceUtility.ExtractStackTrace()}},null,StatsHelper.StatsMask.STATS_BOTH,true);
		}
		if(go==null)Debug.LogError("Resource "+url+" spawn error.");
		if(null!=handler)
			handler.Invoke(go,param);
	}

	//load Object: Object, Sprite, TextAsset etc.
	public IEnumerator load<T>(string url,System.Action<Object> handler){
		Resource res=null;
		try{res=Game.instance.resourceUpdater.All[url];}catch{
			Debug.LogError("Resource "+url+" not found.");
			yield break;
		}
		if(null!=res)yield return StartCoroutine(res.load<T>(handler));
	}

	//load object from TextAsset, especial protocols
	public IEnumerator load(string url,System.Action<object> handler){
		yield return StartCoroutine(load<TextAsset>(url,delegate(Object obj){
			//get string
			Resource res=Game.instance.resourceUpdater.All[url];
			var name=Path.GetFileNameWithoutExtension(res.definition.name);
			var asset=obj as TextAsset;
			byte[] bytes = null;
			bool _b64_encode=true;

			//decode protocol from string
			var msg_type=typeof(World);
			if(name=="world")msg_type=typeof(World);
			else if(name=="creatures")msg_type=typeof(RetrieveCreatureTypeRep);
			try {
				if (_b64_encode)bytes = System.Convert.FromBase64String(asset.text);//from base64
				else bytes = asset.bytes;//raw bytes
				MemoryStream ms = new MemoryStream (bytes);//from bytes
				if(msg_type!=null){
					object o=Game.instance.resourceUpdater.serializer.Deserialize (ms, null, msg_type);//from stream
					handler.Invoke(o);
				}
			} catch (System.Exception e){
				Debug.LogError("Error when load protocol from resources updater: "+name+",error="+e);
			}
		}));
	}

	public bool NeedDownload(string url){
		Resource res=null;
		try{
			res=Game.instance.resourceUpdater.All[url];
			if(Configs.Testing.skipResourceUpdated>0||
				res.definition.type == ResourceType.BUILTINCLIENT||
			   res.IsRequired && (res.state < Resource.State.CACHED)||
			   res.definition.type == ResourceType.OPTIONAL_OVERRIDING && res.state != Resource.State.CACHED||
			   res.definition.type == ResourceType.OPTIONAL && res.state != Resource.State.CACHED||
			   res.www!=null)
				return false;
			else{
				string rurl = DeltaResourceUpdater.MakeUrl(res.definition.name);
				if(DownloadManager.Instance.GetWWW(rurl)!=null)
					return false;
				else if(DownloadManager.Instance.IsBundleCached(rurl,res.definition.name))
					return false;
			}
		}catch(System.Exception e){
			Debug.LogError("Resource "+url+" not found. "+e.ToString());
		}
		return true;
	}

	public void cache(string[] list,int priority=5){
		foreach(var res in list)
			if(Game.instance.resourceUpdater.All.ContainsKey(res))
				Game.instance.resourceUpdater.All[res].cache(priority);
	}

	public static string MakeUrl(string name){
		return name + "." + DownloadManager.Instance.BmConfiger.bundleSuffix;
	}
	public static string MakeName(string url){
		return url.Replace("." + DownloadManager.Instance.BmConfiger.bundleSuffix,"");
	}
}

#if UNITY_EDITOR && !UNITY_WEBPLAYER
public class ResourceListPostprocessor : AssetPostprocessor {
	public static void OnPostprocessAllAssets (string[] importedAssets,string[] deletedAssets, string[] movedAssets,string[] movedFromAssetPaths){
		string path = "Assets/Resources/";
		bool shouldPostProcess = false;
		if(importedAssets==null)return;
		if(deletedAssets==null)return;
		if(movedAssets==null)return;
		if(movedFromAssetPaths==null)return;

		foreach (var str in importedAssets)
		if(str=="ProjectSettings/ProjectSettings.asset"){
			CreateBuildConfig();
			break;
		}

		if(!shouldPostProcess)foreach (string str in importedAssets) {
			if(str.StartsWith(path)){
				shouldPostProcess = true;
				break;
			}
		}
		if(!shouldPostProcess)foreach (string str in deletedAssets) {
			if(str.StartsWith(path)){
				shouldPostProcess = true;
				break;
			}
		}
		if(!shouldPostProcess)foreach (string str in movedAssets) {
			if(str.StartsWith(path)){
				shouldPostProcess = true;
				break;
			}
		}
		if(!shouldPostProcess)foreach (string str in movedFromAssetPaths) {
			if(str.StartsWith(path)){
				shouldPostProcess = true;
				break;
			}
		}
		if (!shouldPostProcess)return;
		CreateRDL();
	}

	public static void CreateBuildConfig(){
		string file = "";
		file += "Testing.buildSource,string,"+System.Environment.UserName+"\n";
		#if UNITY_IPHONE
		file += "Testing.build,string,"+(int.Parse(UnityEditor.PlayerSettings.bundleVersion)).ToString()+"\n";
		#else
		file += "Testing.build,string,"+UnityEditor.PlayerSettings.Android.bundleVersionCode.ToString()+"\n";
		#endif
		file += "Testing.buildTime,string,"+System.DateTime.Now.ToString("yyyy-MM-dd HH:mm")+"\n";
		System.IO.File.WriteAllText ("Assets/Resources/Config/configs_build.csv",file);
	}

	public static void CreateRDL (){
		CreateBuildConfig();
		Edbg.Log("Change in assets detected, recreating BUILTIN resource list.");
		//UnityEditor.PlayerSettings.bundleVersion=(1+int.Parse(UnityEditor.PlayerSettings.bundleVersion)).ToString();
		string file = "";
		file += "using UnityEngine;\n";
		file += "using System.Collections;\n";
		file += "using System.Collections.Generic;\n";
		file += "using gameprotocol.events;\n";
		file += "public class ResourceDefinitionList{\n";
		file += "\tpublic static string version = \""+UnityEditor.PlayerSettings.shortBundleVersion+"\";\n";
		file += "\tpublic static Dictionary<string,ResourceDefinition> GenerateBuiltinResourceList(){\n";
		file += "\n";
		file += "\t\tDictionary<string,ResourceDefinition> lst = new Dictionary<string,ResourceDefinition>();\n";
		file += "\t\tResourceDefinition r = null;\n";
		file += "\n";
		string[] files = Directory.GetFiles ("Assets/Resources/","*.*",SearchOption.AllDirectories);
		List<string> lf=new List<string>();
		foreach(string f in files){
			if(f.Length<=0||f.EndsWith("/")||f.EndsWith(".meta")||f.Contains("/."))continue;
			string ff = f.Replace("Assets/Resources/","#");
			ff = ff.Substring(ff.IndexOf('#')+1,ff.Length-ff.IndexOf('#')-1-(ff.Length-ff.LastIndexOf('.')));
			lf.Add(ff);
		}
		lf.Sort();

		foreach(string ff in lf){
			file += "\t\t\tr = new ResourceDefinition();\n";
			file += "\t\t\tr.type = ResourceType.BUILTINCLIENT;\n";
			file += "\t\t\tr.name = \""+ff+"\";\n";
			file += "\t\t\tlst.Add(r.name, r);\n";
			file += "\n";
		}
		
		file += "\n";
		file += "\t\treturn lst;\n";
		file += "\t}\n";
		file += "}\n";
		
		System.IO.File.WriteAllText ("Assets/Delta/Behaviour/ResourceDefinitionList.cs",file);
		
		Edbg.Log ("\tDone.");
	}

}
#endif
*/
