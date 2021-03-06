var dY = 0.5;
var dX = Math.sqrt(1*1 - 0.5*0.5);

var N = 16;
var cubes = {};
function setCube(u, v, w) {
    if (w == 0)
	delete cubes[u + ',' + v];
    else
	cubes[u + ',' + v] = w;
}
function getCube(u, v) {
    return cubes[u + ',' + v] || 0;
}

function getNeighbors(u, v) {
    // six neighboring cubes, clockwise from top
    return [getCube(u + 1, v - 1),
	    getCube(u + 1, v),
	    getCube(u, v + 1),
	    getCube(u - 1, v + 1),
	    getCube(u - 1, v),
	    getCube(u, v - 1)];
}

function polygon(c, vertices) {
    c.beginPath();
    c.moveTo(vertices[0], vertices[1]);
    for (var i = 2; i < vertices.length; i += 2)
	c.lineTo(vertices[i], vertices[i + 1]);
    c.closePath();
    return c;
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

function fillCube(c, u, v) {
    var w = getCube(u, v);
    if (!w) return;

    c.save();
    c.translate(u + v - 1, v - u - 1);

    var neighbors = getNeighbors(u, v);
    var rhombus;

    if (w >= neighbors[0]) {
	// top
	c.fillStyle = '#ffffff';
	rhombus = [1, 0,
		   2, 1,
		   1, 2,
		   0, 1];
	if (w >= neighbors[5] && w >= neighbors[1])
	    polygon(c, rhombus).fill();
	else if (w >= neighbors[5])
	    polygon(c, leftTriangle(rhombus)).fill();
	else if (w >= neighbors[1])
	    polygon(c, rightTriangle(rhombus)).fill();
    }

    if (w > neighbors[4]) {
	// left
	c.fillStyle = '#555555';
	rhombus = [1, 2,
		   1, 4,
		   0, 3,
		   0, 1];
	if (w >= neighbors[5] && w > neighbors[3])
	    polygon(c, rhombus).fill();
	else if (w >= neighbors[5])
	    polygon(c, leftTriangle(rhombus)).fill();
	else if (w > neighbors[3])
	    polygon(c, rightTriangle(rhombus)).fill();
    }

    if (w >= neighbors[2]) {
	// right
	c.fillStyle = '#aaaaaa';
	rhombus = [1, 2,
		   2, 1,
		   2, 3,
		   1, 4];
	if (w > neighbors[3] && w >= neighbors[1])
	    polygon(c, rhombus).fill();
	else if (w > neighbors[3])
	    polygon(c, leftTriangle(rhombus)).fill();
	else if (w >= neighbors[1])
	    polygon(c, rightTriangle(rhombus)).fill();
    }

    c.restore();
}

function strokeCube(c, u, v) {
    c.save();
    c.translate(u + v - 1, v - u - 1);

    var hexagon = [1, 0,
		   2, 1,
		   2, 3,
		   1, 4,
		   0, 3,
		   0, 1];
    c.strokeStyle = '#aa0000';
    polygon(c, hexagon).stroke();

    c.restore();
}

var mouseU, mouseV;

function draw() {
    var c = $('#canvas')[0].getContext('2d');

    c.clearRect(0, 0, 2 * N, 2 * N);
    for (var v = 0; v < 2*N; v++) {
	for (var u = -N; u < N; u++)
	    fillCube(c, u, v);
    }

    if (mouseU !== undefined && mouseV !== undefined)
	strokeCube(c, mouseU, mouseV);
}

function fromPixel(x, y) {
    x *= N / canvas.width / dX; // = u + v
    y *= N / canvas.height / dY; // = v - u
    return {u: Math.round((x - y) / 2),
	    v: Math.round((x + y) / 2)};
}

function click(e) {
    var p = fromPixel(e.offsetX, e.offsetY);

    var w = getCube(p.u, p.v);
    var neighbors = getNeighbors(p.u, p.v).sort();
    if (w == 0) // create a new cube behind all neighbors
	w = Math.max(1, neighbors[0] - 1); // currently depth must be positive
    else { // move to the next depth
	for (var i = 0; i < 6 && w > neighbors[i]; i++);
	if (i == 6) // we are in front already, so now disappear
	    w = 0;
	else
	    w = neighbors[i] + 1;
    }
    setCube(p.u, p.v, w);
    console.log(p.u + ',' + p.v + ': ' + getCube(p.u, p.v));

    draw();
}

function mousemove(e) {
    var p = fromPixel(e.offsetX, e.offsetY);
    if (mouseU != p.u || mouseV != p.v) {
	console.log(p.u + ',' + p.v + ': ' + getCube(p.u, p.v));

	mouseU = p.u;
	mouseV = p.v;
	draw();
    }
}

for (var v = 0; v < 2*N; v++) {
    for (var u = -N; u < N; u++) {
	if (Math.random() < 0.3)
	    setCube(u, v, Math.ceil(3 * Math.random()));
    }
}

$(function() {
    var canvas = $('#canvas');
    canvas.click(click);
    canvas.mousemove(mousemove);

    canvas = canvas[0];
    var c = canvas.getContext('2d');
    c.scale(canvas.width / N * dX, canvas.height / N * dY);
    c.lineWidth = 2 * N / dX / canvas.width;

    draw();
});
