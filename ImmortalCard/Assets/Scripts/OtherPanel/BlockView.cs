using UnityEngine;
using UnityEngine.UI;
using System.Collections;

public class BlockView : MonoBehaviour {
	public GameObject	Spinner;
	public Text			Info;

	public Text			DlgTitle;
	public Text			DlgContent;
	public GameObject	DlgOk;
	public GameObject	DlgCancel;

	GameObject dlg,mask;
	
	bool blocking=false;
	bool showDialog=false;
	System.Action ok=null;
	System.Action cancel=null;

	static public BlockView Instance;

	void Awake(){
		Instance=this;
		dlg=DlgTitle.transform.parent.gameObject;
		mask=dlg.transform.parent.gameObject;
	}

	void OnDestroy(){
		Instance=null;
	}

	public bool Blocking{
		set{
			blocking=value;
			mask.SetActive(blocking || showDialog);
		}
		get{
			return blocking;
		}
	}

	public bool Slow{
		set{
			Spinner.SetActive(value);
		}
	}

	public void ShowDialog(string content,string title=""
	                       ,System.Action ok=null,System.Action cancel=null){
		DlgContent.text=content;
		if(!string.IsNullOrEmpty(title))
			DlgTitle.text=title;
		this.ok=ok;
		this.cancel=cancel;

		showDialog = true;
		dlg.SetActive(true);
		mask.SetActive(blocking || showDialog);
	}

	public void CloseDialog(){
		showDialog = false;
		dlg.SetActive(false);
		mask.SetActive(blocking || showDialog);
	}

	public void OnOk(){
		if(ok!=null)
			ok.Invoke();
		CloseDialog();
	}

	public void OnCancel(){
		if(cancel!=null)
			cancel.Invoke();
		CloseDialog();
	}
}
