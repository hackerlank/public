using UnityEngine;
using System.Collections;

public class Main : MonoBehaviour {

	public static Main		Instance=null;
	public Player			player=new Player();

	public uint				Round=4;
	public GameController	gameController=null;

	public enum Mode{
		STANDALONE,
		NODE,
		NORMAL,
	}
	public Mode GameMode{get{return Mode.NODE;}}

	void Awake(){
		//Loom
		gameObject.AddComponent<Loom>();
		player=new Player();
		Instance=this;
	}

	void Start () {
	}
	
	void Update () {
	}
}
