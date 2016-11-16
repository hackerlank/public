using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class Main : MonoBehaviour {

	public static Main		Instance=null;

	public const int		Round=2;
	public GameController	gameController=null;
	public ShareAPI			share;

	public Animator			spinner;

	public enum Mode{
		STANDALONE,
		NODE,
		NORMAL,
	}
	public Mode GameMode{get{return Mode.NODE;}}

	void Awake(){
		Instance=this;
		//Loom
		gameObject.AddComponent<Loom>();
		MainPlayer=new Player();
		share=new ShareAPI();
	}

	void Start () {
	}
	
	void Update () {
	}

	public Player MainPlayer=new Player();
	public List<Player> robots=new List<Player>();
}
