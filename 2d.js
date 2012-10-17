var dY = 0.5;
var dX = Math.sqrt(1*1 - 0.5*0.5);

var N = 10;
var cubes = [];
function addCube(u, v) {
    cubes[u + v*N] = true;
}
function removeCube(u, v) {
    delete cubes[u + v*N];
}
function hasCube(u, v) {
    return cubes[u + v*N];
}

function path(c, vertices) {
    c.beginPath();
    c.moveTo(vertices[0], vertices[1]);
    for (var i = 2; i < vertices.length; i += 2)
	c.lineTo(vertices[i], vertices[i + 1]);
    c.closePath();
}

function drawCube(c, u, v) {
    if (!hasCube(u, v)) return;

    c.save();
    c.translate(u + v, v - u);

    if (!hasCube(u + 1, v - 1)) {
	// top
	c.fillStyle = '#ffffff';
	path(c, [1, 0,
		 2, 1,
		 1, 2,
		 0, 1]);
	c.fill();
    }

    if (!hasCube(u - 1, v)) {
	// left
	c.fillStyle = '#555555';
	path(c, [0, 1,
		 1, 2,
		 1, 4,
		 0, 3]);
	c.fill();
    }

    if (!hasCube(u, v + 1)) {
	// right
	c.fillStyle = '#aaaaaa';
	path(c, [2, 1,
		 2, 3,
		 1, 4,
		 1, 2]);
	c.fill();
    }

    c.restore();
}

function draw(c) {
    for (var v = 0; v < N; v++) {
	for (var u = 0; u < N; u++)
	    drawCube(c, u, v);
    }
}

addCube(1, 1);
addCube(1, 2);
addCube(2, 2);
addCube(2, 1);

$(function() {
    var canvas = document.getElementById('canvas');
    var c = canvas.getContext('2d');
    c.scale(canvas.width / N * dX, canvas.height / N * dY);
    draw(c);
});
