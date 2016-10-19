using UnityEngine;
using UnityEngine.UI;
using System.Collections;

public class ChargePanel : MonoBehaviour {

	public GameObject LoginGo,RegisterGo,ChargeGo,QueryGo;
	public GameObject LoginBtn,RegisterBtn,ChargeBtn,QueryBtn,BackBtn;
	//---------------------------------------------------------
	// Login
	//---------------------------------------------------------
	public InputField Phone,Pswd;
	public Toggle IsSave;

	bool registering=false;
	public void OnLogin(){
		if(registering){
			registering=false;
			LoginGo.SetActive(true);
			RegisterGo.SetActive(false);
		}else{
			//login
			LoginGo.SetActive(false);
			RegisterGo.SetActive(false);
			LoginBtn.SetActive(false);
			RegisterBtn.SetActive(false);

			ChargeGo.SetActive(true);
			QueryGo.SetActive(true);
			ChargeBtn.SetActive(true);
			QueryBtn.SetActive(true);
		}
	}

	public void OnFoget(){

	}
	//---------------------------------------------------------
	// Register
	//---------------------------------------------------------
	public InputField RPhone,RPswd;
	public InputField Name,Address,IdCard,Wechat,Alipay,Verify;

	public void OnRegister(){
		if(!registering){
			registering=true;
			LoginGo.SetActive(false);
			RegisterGo.SetActive(true);
		}else{
			//register
		}
	}
	//---------------------------------------------------------
	// Charge
	//---------------------------------------------------------
	public InputField Id,Amount;
	public Text Total;

	bool query=false;
	public void OnCharge(){
		if(query){
			query=false;
			ChargeGo.SetActive(true);
			QueryGo.SetActive(false);
		}else{
			//charge
		}
	}
	//---------------------------------------------------------
	// Query
	//---------------------------------------------------------
	public GridLayoutGroup QueryGrid;

	public void OnQuery(){
		if(!query){
			query=true;
			ChargeGo.SetActive(false);
			QueryGo.SetActive(true);
		}
	}

	public void OnClose(){
		Destroy(gameObject);
	}
}
