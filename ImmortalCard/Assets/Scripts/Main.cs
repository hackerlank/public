using UnityEngine;
using System.Collections;

public class Main : MonoBehaviour {

	public static Main	Instance=null;
	public HttpProxy	http;
	public WSProxy		ws;

	void Awake(){
		http=new HttpProxy();
		ws=new WSProxy();

		Instance=this;
	}

	void Start () {

	}
	
	void Update () {
	
	}
}
