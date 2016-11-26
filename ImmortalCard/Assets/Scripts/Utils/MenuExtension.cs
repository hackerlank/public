using UnityEngine;
using System.Collections;

#if UNITY_EDITOR
using UnityEditor;

public class MenuExtension {

	[MenuItem("Tools/ClearPlayerPrefs")]
	private static void NewMenuOption()
	{
		PlayerPrefs.DeleteAll();
	}
}

#endif
