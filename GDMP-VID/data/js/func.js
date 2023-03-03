
let semaine_type = ["Désactivé", "7/7 jours", "Sam. -> Dim.", "Lun. -> Ven."];
// fonction de mise à jour des datas et de la timestamp par ajax
function maj_value(type, name_btn = 'null') {
	let url = "./maj.json";
	let ajaxRequest = new XMLHttpRequest();
	ajaxRequest.onreadystatechange = function () {
		if (ajaxRequest.readyState == 4) {
			//si la requete est complete
			if (ajaxRequest.status == 200) {
				let res = JSON.parse(ajaxRequest.response);
				for (var key in res) {
					// on met a jour les ID 
					if (key.indexOf("semaine") !== -1) { // si le tag contient "semaine" on change le texte
						document.querySelector('#' + key).innerHTML = semaine_type[res[key]];
					}
					else if (key.indexOf("duration") !== -1) { // si le tag contient "duration" on change le texte
						document.querySelector('#' + key).innerHTML = minute_txt(res[key]);
					}
					else {
						// on met a jour les ID 
						document.querySelector('#' + key).innerHTML = res[key];
					}
				}
			}
		}
	}
	
	if (type === 'time') {// si time est à true on envoie la timestamp à la carte
		let date = new Date();
		let timestamp = Math.floor((date.getTime() - (date.getTimezoneOffset() * 60000)) / 1000);
		url = './maj.json?timestamp=' + timestamp;
	}
	else if (type === 'btn' && name_btn != 'null') {// si btn est présent chargement du mode de commande pour la page cdm_btn.html
		url = './maj.json?btn=' + name_btn;

	}
	else if (type === 'cdm' && name_btn != 'null') {// si cdm est present changement d'etat de mode de commande
		url = './maj.json?cdm=' + name_btn;

	}
	else if (type === 'config' && name_btn != 'null') {// si config est présent on envoie la config des programmation pour la page cdm_btn.html
		url = './config/' + name_btn + '.json';

	}
	else { url = "./maj.json"; } // si rien on affiche l'etat des capteur
	ajaxRequest.open('GET', url);
	ajaxRequest.send();
}
/********************************************************/

// fonction de mise à jour de l'état du mode de la page de cdm_btn
/*function maj_cdm_btn(btn) {
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
}*/

// fonction de chargement du fichier d'historique
function maj_histo(name_btn, cdm = 'null') {
	let url = "./histo/";
	let ajaxRequest = new XMLHttpRequest();
	ajaxRequest.onreadystatechange = function () {
		if (ajaxRequest.readyState == 4) {
			//si la requete est complete
			if (ajaxRequest.status == 200) {// on affiche les historique
				document.querySelector('#histo').innerHTML = ajaxRequest.responseText.replace(/\n/g, '<br />\n');
			}
		}
	}
	if (cdm === 'del' && name_btn != null) { // command del on efface les historique
		url += '?btn=' + name_btn;

	}
	else {// sinon on affiche seullement
		url += name_btn + ".txt";
	}
	ajaxRequest.open('GET', url);
	ajaxRequest.send();
}


// fonction mise en forme texte durer
function minute_txt(minutes) {
	let retour;
	let mm = minutes % 60;
	let h = (minutes - mm) / 60;
	if (mm == 0) { mm = "0" + mm }
	if (h < 1) {
		if (mm == 0) { retour = "Off" }
		else { retour = mm + " Min"; }
	}
	else { retour = h + "H" + mm; }
	return retour;
}
// function de création des option des select 
function option_txt() {

	let heure_txt;
	let id_txt;
	id_txt = "on";
	for (let select = 1; select <= 4; select++) {
		id = id_txt.concat('_', select);
		for (let heure = 0; heure <= 23; heure++) {
			if (heure <= 9) {
				heure_txt = "0" + heure;
			}
			else { heure_txt = heure; }
			document.querySelector('#' + id).options[heure] = new Option(heure_txt + " H", heure);
		}
	}
	id_txt = "duration";
	for (let select = 1; select <= 4; select++) {
		id = id_txt.concat('_', select);
		let i = 0;
		for (let durer = 0; durer <= 240; durer += 15) {
			document.querySelector('#' + id).options[i] = new Option(minute_txt(durer), durer);
			i++;
		}
	}

	id_txt = "semaine";
	for (let select = 1; select <= 4; select++) {
		id = id_txt.concat('_', select);
		for (let semaine = 0; semaine <= 3; semaine++) {
			document.querySelector('#' + id).options[semaine] = new Option(semaine_type[semaine], semaine);
		}
	}
}

// fonction de mise à jour de l'état des select option pour la configuration
function maj_config(name_btn, save = false) {
	let url = './config/' + name_btn + '.json';
	let ajaxRequest = new XMLHttpRequest();
	ajaxRequest.onreadystatechange = function () {
		if (ajaxRequest.readyState == 4) {
			//si la requete est complete
			if (ajaxRequest.status == 200) {
				let res = JSON.parse(ajaxRequest.response);
				for (var key in res) {
					// on met a jour les ID des option des select
					document.querySelector('#' + key).value = res[key];
				}
			}
		}
	}
	if (save == true) { // si save est present on envoie les valeur des option
		// on extrait les donnée des select
		let get_on = "on";
		let get_duration = "duration";
		let get_semaine = "semaine";
		let get ="";
		for (let i = 1; i <= 4; i++) {
			let id_on = get_on.concat('_', i)
			let value_on = document.querySelector('#' + id_on).value;
			get += "&" + id_on + "=" + value_on;

			let id_duration = get_duration.concat('_', i)
			let value_duration = document.querySelector('#' + id_duration).value;
			get += "&" + id_duration + "=" + value_duration;

			let id_semaine = get_semaine.concat('_', i)
			let value_semaine = document.querySelector('#' + id_semaine).value;
			get += "&" + id_semaine + "=" + value_semaine;
		}
		url = './config/?save_btn=' + name_btn + get;
	}

	ajaxRequest.open('GET', url);
	ajaxRequest.send();
}
