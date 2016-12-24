﻿using UnityEngine;
using UnityEngine.UI;
using System.Collections;

public class Option : MonoBehaviour {

	public Toggle		toggle;
	public Text			label;
	public Image		checker;
	public Image		checkerMarker;
	public Image		radio;
	public Image		radioMarker;

	[HideInInspector]
	public ToggleGroup	group;

	void Start(){
		toggle.targetGraphic=checker;
		toggle.graphic=checkerMarker;
	}

	Hashtable hash;
	public Hashtable Value{
		set{
			hash=value;

			label.text=value["label"] as string;
		}
		get{
			return hash;
		}
	}
}