// fonction de mise a jour des datas par ajax
function maj_value() {
	var ajaxRequest = new XMLHttpRequest();
	ajaxRequest.onreadystatechange = function () {
		if (ajaxRequest.readyState == 4) {
			//si la requete est complete
			if (ajaxRequest.status == 200) {
				let res = JSON.parse(ajaxRequest.response);
				for (var key in res) {
					// on met a jour les ID texte du svg
					document.getElementById(key).innerHTML = res[key];
				}
			}
		}
	}
	ajaxRequest.open('GET', './maj.json');
	ajaxRequest.send();
}