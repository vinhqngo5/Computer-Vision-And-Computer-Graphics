const GAME = {
	MAX_VELOCITY: 20,
	DEFAULT_VELOCITY: 10,
};

let nextLevel = "index.html";

const shoot = (velocity = GAME.DEFAULT_VELOCITY) => {
	const bullet = document.createElement("a-sphere");
	let pos = myCamera.getAttribute("position");
	bullet.setAttribute("position", pos);
	bullet.setAttribute("shadow", "true");
	bullet.setAttribute("velocity", getDirection(myCamera, velocity));
	bullet.setAttribute("dynamic-body", true);
	bullet.setAttribute("radius", 0.2);
	bullet.setAttribute("src", "https://i.imgur.com/cBUjdOb.png");
	myScene.appendChild(bullet);
	bullet.addEventListener("collide", shootCollided);
};

const shootCollided = (event) => {
	if (event.detail.body.el.id === "floor") {
		event.detail.target.el.removeEventListener("collide", shootCollided);
	} else if (event.detail.body.el.className === "target") {
		console.log("Hit the target");
		event.detail.target.el.removeEventListener("collide", shootCollided);
		myScene.removeChild(event.detail.body.el);
	}
	if (document.querySelectorAll(".target").length === 0) {
		console.log("You win");
		location.href = nextLevel;
	}
};

const shootInDesktop = (el, callback) => {
	let touchsurface = el,
		elapsedTime,
		startTime,
		handlePress = callback || function (swipeDir) {};

	touchsurface.addEventListener(
		"keydown",
		function (e) {
			if (e.repeat) {
				return; // Event fired when press too long
			}
			startTime = new Date().getTime(); // record time when finger first makes contact with surface
			e.preventDefault();
		},
		false
	);

	touchsurface.addEventListener(
		"keyup",
		function (e) {
			elapsedTime = new Date().getTime() - startTime; // get time elapsed
			handlePress(e, Math.min(elapsedTime / 20, GAME.MAX_VELOCITY));
			e.preventDefault();
		},
		false
	);
};

function swipeDetect(el, callback) {
	var touchsurface = el,
		swipeDir,
		startX,
		startY,
		distX,
		distY,
		threshold = 30, //required min distance traveled to be considered swipe
		restraint = 100, // maximum distance allowed at the same time in perpendicular direction
		allowedTime = 300, // maximum time allowed to travel that distance
		elapsedTime,
		startTime,
		handleswipe = callback || ((swipeDir, velocity) => {});

	touchsurface.addEventListener(
		"touchstart",
		function (e) {
			var touchobj = e.changedTouches[0];
			swipeDir = "none";
			dist = 0;
			startX = touchobj.pageX;
			startY = touchobj.pageY;
			startTime = new Date().getTime(); // record time when finger first makes contact with surface
			e.preventDefault();
		},
		false
	);

	touchsurface.addEventListener(
		"touchmove",
		function (e) {
			e.preventDefault(); // prevent scrolling when inside DIV
		},
		false
	);

	touchsurface.addEventListener(
		"touchend",
		function (e) {
			var touchobj = e.changedTouches[0];
			distX = touchobj.pageX - startX; // get horizontal dist traveled by finger while in contact with surface
			distY = touchobj.pageY - startY; // get vertical dist traveled by finger while in contact with surface
			elapsedTime = new Date().getTime() - startTime; // get time elapsed
			if (elapsedTime <= allowedTime) {
				// first condition for awipe met
				if (Math.abs(distX) >= threshold && Math.abs(distY) <= restraint) {
					// 2nd condition for horizontal swipe met
					swipeDir = distX < 0 ? "left" : "right"; // if dist traveled is negative, it indicates left swipe
				} else if (
					Math.abs(distY) >= threshold &&
					Math.abs(distX) <= restraint
				) {
					// 2nd condition for vertical swipe met
					swipeDir = distY < 0 ? "up" : "down"; // if dist traveled is negative, it indicates up swipe
				}
			}
			handleswipe(swipeDir, Math.min(Math.abs(distY / 8), GAME.MAX_VELOCITY));
			e.preventDefault();
		},
		false
	);
}

shootInDesktop(document, function (event, velocity) {
	if (event.which == 32) shoot(velocity);
});
swipeDetect(document, function (swipeDir, velocity) {
	if (swipeDir == "up") shoot(velocity);
});
