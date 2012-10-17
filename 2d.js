var dY = 0.5;
var dX = Math.sqrt(1*1 - 0.5*0.5);

var N = 10;
var cubes = [];
function addCube(u, v, w) {
    cubes[u + v*N] = w;
}
function removeCube(u, v) {
    delete cubes[u + v*N];
}
function getCube(u, v) {
    return cubes[u + v*N] || 0;
}

function fillPolygon(c, vertices) {
    c.beginPath();
    c.moveTo(vertices[0], vertices[1]);
    for (var i = 2; i < vertices.length; i += 2)
	c.lineTo(vertices[i], vertices[i + 1]);
    c.closePath();
    c.fill();
}

function drawCube(c, u, v) {
    var w = getCube(u, v);
    if (!w) return;

    c.save();
    c.translate(u + v, v - u);

    if (w >= getCube(u + 1, v - 1)) {
	c.fillStyle = '#ffffff';
	if (w >= getCube(u, v - 1)) // top-left
	    fillPolygon(c, [1, 0,
			    0, 1,
			    1, 2]);
	if (w >= getCube(u + 1, v)) // top-right
	    fillPolygon(c, [1, 0,
			    2, 1,
			    1, 2]);
    }

    if (w > getCube(u - 1, v)) {
	c.fillStyle = '#555555';
	if (w >= getCube(u, v - 1)) // left-top
	    fillPolygon(c, [0, 1,
			    1, 2,
			    0, 3]);
	if (w > getCube(u - 1, v + 1)) // left-bottom
	    fillPolygon(c, [1, 2,
			    0, 3,
			    1, 4]);
    }

    if (w > getCube(u, v + 1)) {
	c.fillStyle = '#aaaaaa';
	if (w >= getCube(u + 1, v)) // right-top
	    fillPolygon(c, [2, 1,
			    1, 2,
			    2, 3]);
	if (w > getCube(u - 1, v + 1)) // right-bottom
	    fillPolygon(c, [1, 2,
			    2, 3,
			    1, 4]);
    }

    c.restore();
}

function draw(c) {
    for (var v = 0; v < N; v++) {
	for (var u = 0; u < N; u++)
	    drawCube(c, u, v);
    }
}

addCube(1, 3, 2);
addCube(1, 4, 1);
addCube(2, 4, 2);
addCube(2, 3, 3);

$(function() {
    var canvas = document.getElementById('canvas');
    var c = canvas.getContext('2d');
    c.scale(canvas.width / N * dX, canvas.height / N * dY);
    draw(c);
});
