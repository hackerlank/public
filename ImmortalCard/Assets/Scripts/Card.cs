using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;

public class Card : MonoBehaviour,IPointerClickHandler,IDragHandler,IBeginDragHandler,IEndDragHandler {

	void Start () {
	
	}
	
	void Update () {
	
	}

	public void OnPointerClick (PointerEventData eventData){
		if(eventData==null||eventData.dragging||eventData.pointerEnter==null)return;
		Debug.Log("----click on card");
	}
	
	bool dragging=false;
	Vector2 dragFrom=Vector2.zero;
	public void OnBeginDrag (PointerEventData eventData){
		dragging=true;
		dragFrom=transform.position;
		//Debug.Log("----begin from "+dragFrom.ToString());
	}
	
	public void OnEndDrag (PointerEventData eventData){
		dragging=false;
		var lg=transform.parent.GetComponent<HorizontalLayoutGroup>();
		lg.CalculateLayoutInputVertical();
		lg.CalculateLayoutInputHorizontal();

		//restrore
		//transform.position=dragFrom;
		DiscardTo(GamePanel.Instance.LDiscardArea);
		//Debug.Log("----end drag");
	}
	
	public void OnDrag(PointerEventData eventData){
		//Debug.Log("----drag on card");
		var delta=eventData.delta;
		eventData.pointerDrag.transform.position+=new Vector3(delta.x,delta.y,0);
	}

	public void DiscardTo(Transform group){
		transform.SetParent(group);
	}
}
