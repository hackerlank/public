using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Reflection;

public class Updater : MonoBehaviour {

	private bool isResourceUpdatingReq = false;
	private bool isResourceUpdatingOpt = false;
	public bool SyncLoading = true;
	public bool SkipUpdate = false;

	void Start () {
		SetupProgressBar (0f);
	}

	void Update () {
		SetProgressBar(RequiredDownloadingProgress());
	}

	public void SetProgressBar(float progress){
		if (isResourceUpdatingReq || isResourceUpdatingOpt)
			return;
		SetupProgressBar (progress);
	}

	private void SetupProgressBar(float progress){
	}

	public IEnumerator BindProgressBar(WWW www)
	{
		while (!www.isDone) {
			SetProgressBar (0.5f);
			yield return null;
		}
		SetProgressBar (0f);
	}

	public void AddResource(string name,WWW www){
		//AddResource(res);
	}

	public float RequiredDownloadingProgress(){
		float total = 0;
		float finished = 0;
		/*
		foreach (Resource r in Required) {
			total+=1f;
			if(r.state >= Resource.State.CACHED) finished+=1f;
			else if((r.www!=null)&&(r.www.error!=null))finished += r.www.progress;
		}
		*/
		if (total == 0)
			return 1f;
		return finished / total;
	}

	public void cache(string uri){
		string url = Updater.MakeUrl(uri);
		StartCoroutine(DownloadManager.Instance.WaitDownload(url,10));
	}

	public IEnumerator Load<T>(string uri,Transform parent=null,System.Action<Object,Hashtable> handler=null,Hashtable param=null){
		if(!SkipUpdate)
			while(!DownloadManager.Instance.ConfigLoaded)
				yield return null;

		Object lo=null;
		//load from cache or remote
		if(DownloadManager.Instance.IsBundleExists(uri)){
			WWW www = null;			
			string url = Updater.MakeUrl(uri);
			while(www==null){
				yield return StartCoroutine(DownloadManager.Instance.WaitDownload(url,10));
				www = DownloadManager.Instance.GetWWW(url);
			}
			if(www != null){
				var name=Path.GetFileNameWithoutExtension(uri);
				lo=www.assetBundle.Load(name,typeof(T));
				if(lo!=null)lo = Instantiate(lo,name);
				www.assetBundle.Unload (false);
				DownloadManager.Instance.DisposeWWW(url);	//www.Dispose ();
				www = null;
			}
		}

		//load from local
		if(lo==null){
			ResourceRequest req=null;
			if(uri.Contains("Sprite/")){
				req = Resources.LoadAsync(uri,typeof(Sprite));
				while(!req.isDone)yield return null;
				lo=Instantiate (req.asset as Sprite,uri);
			}else{
				req = Resources.LoadAsync(uri,typeof(T));
				while(!req.isDone)yield return null;
				lo=Instantiate (req.asset,uri);
			}
		}

		//parent
		if(parent!=null && lo is Component)
			(lo as Component).transform.SetParent(parent,false);

		//callback
		if(handler!=null)
			handler.Invoke(lo,param);
	}

	Object Instantiate(Object obj,string objname){
		if(obj==null){
			return null;
		}else
			return ((obj is Sprite)||(obj is Texture2D))?obj:
				MonoBehaviour.Instantiate(obj);
	}

	public bool NeedDownload(string url){
		/*
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
		*/
		return true;
	}

	public static string MakeUrl(string name){
		BMConfiger bmConfiger=new BMConfiger();
		return name + "." + bmConfiger.bundleSuffix;
	}
	public static string MakeName(string url){
		BMConfiger bmConfiger=new BMConfiger();
		return url.Replace("." + bmConfiger.bundleSuffix,"");
	}
}
