using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;
using Proto3;

public class Bunch : MonoBehaviour{
	public Transform		Cards;

	public System.Action<Bunch> onTap=delegate(Bunch bunch){};

	protected bunch_t _bunch=null;
	virtual public bunch_t Value{
		get{
			return _bunch;
		}
		set{
			_bunch=value;
		}
	}

	public void OnPUP(){
		if(onTap!=null)onTap.Invoke(this);
	}
}
