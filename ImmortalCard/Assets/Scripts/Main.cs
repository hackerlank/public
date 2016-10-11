using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class Main : MonoBehaviour {

	public static Main		Instance=null;

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
		MainPlayer=new Player();
		Instance=this;
	}

	void Start () {
	}
	
	void Update () {
	}

	public Player MainPlayer=new Player();
	public List<Player> players=new List<Player>();
	public Player GetPlayer(int index){
		return (index<players.Count?players[index]:null);
	}
}
