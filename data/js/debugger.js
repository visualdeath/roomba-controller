var currentUrl = window.location.host;
//var currentUrl = "192.168.86.146";
var socket;

function connect() {
	socket = new WebSocket("ws://" + currentUrl + ":8232");
	socket.onopen = function (event) {
		addConsole( 'Connection Established' );
		console.log("WebSocket is open now.");
		sendStr($('input[type=radio][name=logLevel]:checked').val(), false);
	}

	socket.onmessage = function (event) {
		if ( event.data.substring(0, 5) == '$app:' ) {
			return;
		} else if ( (event.data.substring(0,1) == '*') || (event.data.substring(0,4) == '[0m*') ) {
			addSystem( event.data );
		} else {
			addMessage( event.data );
		}
	}

	socket.onclose = function (e) {
		addConsole( 'Connection Closed: ' + e.code + ':' + e.reason );
		console.log('WebSocket closed: ', e);
		console.log('Reconnect will be attempted in 1 second.', e.reason);
		setTimeout(function() {
		  connect();
		}, 1000);
	}

	socket.onerror = function (err) {
		addError( 'Error: ' + err.message );
		console.error('Socket encountered error: ', err.message, 'Closing socket');
		ws.close();
	}
}

$("body").on('DOMSubtreeModified', "#terminal", function() {
	if ($("#autoScroll").prop('checked')!=false) {
		$("#terminal").scrollTop($("#terminal")[0].scrollHeight);
	}
});

$('input[type=radio][name=logLevel]').change(function() {
	if (socket.readyState == 1) {
		sendStr(this.value, false);
	}
});

$('#message').keydown(function(event) {
	if (event.which == 13) {
		sendCmd();
	}
});

$('#send').click(function() {
	sendCmd();
});

function addMessage(s) {
	if (s.length > 0) {
		var msg = ansiColor(s);
		//var msg = s;
		$("#terminal").append( '<p class="message">' + msg + '</p>' );
	}
}

function addError(s) {
	if (s.length > 0) {
		$("#terminal").append( '<p class="error">(ERROR) ' + s + '</p>' );
	}
}

function addConsole(s) {
	if (s.length > 0) {
		$("#terminal").append( '<p class="console">(CONSOLE) ' + s + '</p>' );
	}
}

function addSystem(s) {
	if (s.length > 0) {
		var msg = cleanStr(s);
		$("#terminal").append( '<p class="system">(SYSTEM) ' + msg + '</p>' );
	}
}

function addSent(s) {
	if (s.length > 0) {
		$("#terminal").append( '<p class="sent">(SENT) ' + s + '</p>' );
	}
}

function sendCmd() {
	if (socket.readyState == 1) {
		var cmd = $('#message');
		if (cmd.val().length > 0) {
			sendStr(cmd.val());
			cmd.val('');
		}
	}
}

function sendStr(s, d=true) {
	if (socket.readyState == 1) {
		if (s.length > 0) {
			socket.send(s);
			if (d == true) {
				addSent(s);
			}
		}	
	}
}

function cleanStr(s) {
	return s.replaceAll(/\[[0-9;]+m/gm, '');
}

var globalColorModifiers = [];
function ansiColor(s) {
	//debugger;
	const needle = /\[([0-9;]+)m/m;
	var result = s;
	var openTags = 0;
	var x = -1;
	
	while ( result.search(needle) != -1 ) {
		x = result.search(needle) + 1;
		var val = "";
		var chr = "";
		var tags = "";
		while ( (chr = result.charAt(x)) != 'm' ) {
			val += chr;
			x++;
		}
		if (val.indexOf(';') != -1) {
			var tmp = val.split(';')
			tmp.forEach(function(v) {
				processAnsiModifier(v);
				if (v == '0' && openTags > 0) {
					tags += '</span>';
					openTags--;
				}
			});
			if (globalColorModifiers.length > 0) {
				tags += '<span class="' + getModifierClass() + '">'
				openTags++;
			}
		} else {
			processAnsiModifier(val);
			if (val == '0' && openTags > 0) {
				tags += '</span>';
				openTags--;
			} else {
				tags += '<span class="' + getModifierClass() + '">'
				openTags++;
			}
		}
		result = result.replace(needle, tags);
	}
	
	if (openTags > 0) {
		for ( let i = 0; i < openTags; i++ ) {
			result += '</span>';
		}
	}
	
	return result;
}

function getModifierClass() {
	var result = '';
	globalColorModifiers.forEach(function(v) {
		result += 'ansi-' + v + ' ';
	});
	return result.trim();
}

function processAnsiModifier(s) {
	switch(s) {
		case '0':
			globalColorModifiers = [];
			break;
		case '21':
			// Bold Off 1
			removeModifier('1');
			break;
		case '22':
			// Bold and faint Off 1 2 
			removeModifier('1');
			removeModifier('2');
			break;
		case '23':
			// italic Off 3
			removeModifier('3');
			break;
		case '24':
			// Underline Off 4
			removeModifier('4');
			break;
		case '25':
			// Blink Off 5 6
			removeModifier('5');
			removeModifier('6');
			break;
		case '27':
			// Inverse Off 7
			removeModifier('7');
			break;
		case '28':
			// Conceal Off 8
			removeModifier('8');
			break;
		case '29':
			// Crossed Out Off 9
			removeModifier('9');
			break;
		default:
			globalColorModifiers.push(s);
			break;
	}
}

function removeModifier(v) {
	const index = globalColorModifiers.indexOf(v);
	if (index > -1) {
		globalColorModifiers.splice(index, 1);
	}
}

connect();