using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class Option : MonoBehaviour {

	public Toggle		toggle;
	public Text			label;
	public Image		checker;
	public Image		checkerMarker;
	public Image		radio;
	public Image		radioMarker;

	Hashtable hash;
	key_value kv;

	public Hashtable Value{
		set{
			hash=value;

			label.text=value["label"] as string;

			foreach(var _key in value.Keys){
				var key=_key as string;
				if(key!="label" && key!="group"){
					kv=new key_value();
					kv.Key=key;
					kv.Ivalue=System.Convert.ToInt32(value[key]);
					break;
				}
			}
		}
		get{
			return hash;
		}
	}

	public key_value KeyValue{
		get{
			return kv;
		}
	}
}
