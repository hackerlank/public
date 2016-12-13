using System;
using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;
using Object = UnityEngine.Object;

public class EditorCoroutine
{
	public static EditorCoroutine start( IEnumerator _routine )
	{
		EditorCoroutine coroutine = new EditorCoroutine(_routine);
		coroutine.start();
		return coroutine;
	}
	
	readonly IEnumerator routine;
	EditorCoroutine( IEnumerator _routine )
	{
		routine = _routine;
	}
	
	void start()
	{
		//Debug.Log("start");
		EditorApplication.update += update;
	}
	public void stop()
	{
		//Debug.Log("stop");
		EditorApplication.update -= update;
	}

	WWW www = null;
	void update()
	{
		/* NOTE: no need to try/catch MoveNext,
			 * if an IEnumerator throws its next iteration returns false.
			 * Also, Unity probably catches when calling EditorApplication.update.
			 */
		
		//Debug.Log("update");
		if ((www != null) && (!www.isDone)) {
			//Debug.Log("Editor Waiting for WWW.");
			return;
		}
		if ((www != null) && (www.isDone))Debug.Log ("WWW finished.");
		www = null;
		if (!routine.MoveNext())
		{
			stop();
		}
		if (routine.Current is WWW) {
			www = (WWW)routine.Current;
		}
	}
}
