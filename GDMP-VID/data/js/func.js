// fonction de mise à jour des datas et de la timestamp par ajax
function maj_value(type,name_btn = 'null') {
	let url = "./maj.json";
	let ajaxRequest = new XMLHttpRequest();
	ajaxRequest.onreadystatechange = function () {
		if (ajaxRequest.readyState == 4) {
			//si la requete est complete
			if (ajaxRequest.status == 200) {
				let res = JSON.parse(ajaxRequest.response);
				for (var key in res) {
					// on met a jour les ID texte du svg
					document.querySelector('#' + key).innerHTML = res[key];
				}
			}
		}
	}
	// si time est à true on envoie la timestamp à la carte
	if (type === 'time') {
		let date = new Date();
		let timestamp = Math.floor((date.getTime() - (date.getTimezoneOffset() *60000))/1000);
		url += '?timestamp='+timestamp;
	}
	else if (type === 'btn' && name_btn !='null'){
		url += '?btn=' + name_btn;

	}
	else if (type === 'cdm' && name_btn !='null'){
		url += '?cdm=' + name_btn;

	}
	ajaxRequest.open('GET', url);
	ajaxRequest.send();
}
/********************************************************/

// fonction de mise à jour des datas de la page de cdm_btn
function maj_cdm_btn(btn) {
	let url = "./maj_btn.json?btn=" + btn;
	let ajaxRequest = new XMLHttpRequest();
	ajaxRequest.onreadystatechange = function () {
		if (ajaxRequest.readyState == 4) {
			//si la requete est complete
			if (ajaxRequest.status == 200) {
				let res = JSON.parse(ajaxRequest.response);
				for (var key in res) {
					// on met a jour les ID texte du svg
					document.querySelector('#' + key).innerHTML = res[key];
				}
			}
		}
	}
	ajaxRequest.open('GET', url);
	ajaxRequest.send();
}
