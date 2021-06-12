const AFRAME = window.AFRAME;

/*
 * Scales the object proportionally to a set value given in meters.
 */

AFRAME.registerComponent("natural-size", {
	schema: {
		width: {
			type: "number",
			default: undefined, // meters
		},
		height: {
			type: "number",
			default: undefined, // meters
		},
		depth: {
			type: "number",
			default: undefined, // meters
		},
	},

	init() {
		this.el.addEventListener("model-loaded", this.rescale.bind(this));
	},

	update() {
		this.rescale();
	},

	rescale() {
		const el = this.el;
		const data = this.data;
		const model = el.object3D;

		const box = new THREE.Box3().setFromObject(model);
		const size = box.getSize();

		if (!size.x && !size.y && !size.z) {
			return;
		}

		let scale = 1;

		let { x, y, z } = el.getAttribute("position");
		scale = (y * 2) / size.y;
		el.setAttribute("scale", `${scale} ${scale} ${scale}`);
		console.log("dcm");
		console.log(scale);
	},

	remove() {
		this.el.removeEventListener("model-loaded", this.rescale);
	},
});
