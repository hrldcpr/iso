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

function leftTriangle(r) {
    return [r[0], r[1],
	    r[4], r[5],
	    r[6], r[7]];
}

function rightTriangle(r) {
    return [r[0], r[1],
	    r[2], r[3],
	    r[4], r[5]];
}

function drawCube(c, u, v) {
    var w = getCube(u, v);
    if (!w) return;

    c.save();
    c.translate(u + v, v - u);

    // six neighboring cubes, clockwise from top
    var neighbors = [getCube(u + 1, v - 1),
		     getCube(u + 1, v),
		     getCube(u, v + 1),
		     getCube(u - 1, v + 1),
		     getCube(u - 1, v),
		     getCube(u, v - 1)];
    var rhombus;

    if (w >= neighbors[0]) {
	// top
	c.fillStyle = '#ffffff';
	rhombus = [1, 0,
		   2, 1,
		   1, 2,
		   0, 1];
	if (w >= neighbors[5] && w >= neighbors[1])
	    fillPolygon(c, rhombus);
	else if (w >= neighbors[5])
	    fillPolygon(c, leftTriangle(rhombus));
	else if (w >= neighbors[1])
	    fillPolygon(c, rightTriangle(rhombus));
    }

    if (w > neighbors[4]) {
	// left
	c.fillStyle = '#555555';
	rhombus = [1, 2,
		   1, 4,
		   0, 3,
		   0, 1];
	if (w >= neighbors[5] && w > neighbors[3])
	    fillPolygon(c, rhombus);
	else if (w >= neighbors[5])
	    fillPolygon(c, leftTriangle(rhombus));
	else if (w > neighbors[3])
	    fillPolygon(c, rightTriangle(rhombus));
    }

    if (w >= neighbors[2]) {
	// right
	c.fillStyle = '#aaaaaa';
	rhombus = [1, 2,
		   2, 1,
		   2, 3,
		   1, 4];
	if (w > neighbors[3] && w >= neighbors[1])
	    fillPolygon(c, rhombus);
	else if (w > neighbors[3])
	    fillPolygon(c, leftTriangle(rhombus));
	else if (w >= neighbors[1])
	    fillPolygon(c, rightTriangle(rhombus));
    }

    c.restore();
}

function draw(c) {
    for (var v = 0; v < N; v++) {
	for (var u = 0; u < N; u++)
	    drawCube(c, u, v);
    }
}

addCube(1, 3, 1);
addCube(1, 4, 1);
addCube(2, 4, 2);
addCube(2, 3, 1);

$(function() {
    var canvas = document.getElementById('canvas');
    var c = canvas.getContext('2d');
    c.scale(canvas.width / N * dX, canvas.height / N * dY);
    draw(c);
});
