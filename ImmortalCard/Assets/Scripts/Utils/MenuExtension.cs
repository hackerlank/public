using UnityEngine;
using UnityEditor;
using System.Collections;

public class MenuExtension {

	[MenuItem("Tools/ClearPlayerPrefs")]
	private static void NewMenuOption()
	{
		PlayerPrefs.DeleteAll();
	}
}
