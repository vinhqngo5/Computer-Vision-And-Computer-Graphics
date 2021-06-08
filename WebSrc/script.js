const GAME = {
	MAX_VELOCITY: 20,
	DEFAULT_VELOCITY: 10,
};

const shoot = (velocity = GAME.DEFAULT_VELOCITY) => {
	const bullet = document.createElement("a-sphere");
	let pos = myCamera.getAttribute("position");
	bullet.setAttribute("position", pos);
	bullet.setAttribute("velocity", getDirection(myCamera, velocity));
	bullet.setAttribute("dynamic-body", true);
	bullet.setAttribute("radius", 0.2);
	bullet.setAttribute("src", "https://i.imgur.com/H8e3Vnu.png");
	myScene.appendChild(bullet);
	bullet.addEventListener("collide", shootCollided);
};

const shootCollided = (event) => {
	console.log("Hit something");
};

const shootInDesktop = (el, callback) => {
	let touchsurface = el,
		elapsedTime,
		startTime,
		handlePress = callback || function (swipedir) {};

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
			console.log(new Date().getTime());
			handlePress(e, Math.min(elapsedTime / 20, GAME.MAX_VELOCITY));
			e.preventDefault();
		},
		false
	);
};

function swipedetect(el, callback) {
	var touchsurface = el,
		swipedir,
		startX,
		startY,
		distX,
		distY,
		threshold = 30, //required min distance traveled to be considered swipe
		restraint = 100, // maximum distance allowed at the same time in perpendicular direction
		allowedTime = 300, // maximum time allowed to travel that distance
		elapsedTime,
		startTime,
		handleswipe = callback || ((swipedir, velocity) => {});

	touchsurface.addEventListener(
		"touchstart",
		function (e) {
			var touchobj = e.changedTouches[0];
			swipedir = "none";
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
					swipedir = distX < 0 ? "left" : "right"; // if dist traveled is negative, it indicates left swipe
				} else if (
					Math.abs(distY) >= threshold &&
					Math.abs(distX) <= restraint
				) {
					// 2nd condition for vertical swipe met
					swipedir = distY < 0 ? "up" : "down"; // if dist traveled is negative, it indicates up swipe
				}
			}
			handleswipe(swipedir, Math.min(Math.abs(distY / 8), GAME.MAX_VELOCITY));
			e.preventDefault();
		},
		false
	);
}

shootInDesktop(document, function (event, velocity) {
	if (event.which == 32) shoot(velocity);
});
swipedetect(document, function (swipedir, velocity) {
	if (swipedir == "up") shoot(velocity);
});
