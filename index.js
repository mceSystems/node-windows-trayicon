const nbind = require("nbind");
const { lib: { CTrayIconContainer } } = nbind.init(__dirname);

class WindowsTryicon {
	constructor(options) {
		this.__itemCallbacks = [];
		this.__iconPath = options.icon || "";
		this.__menuItems = options.menu || [];

		this.__nativeTray = new CTrayIconContainer();
		for(const item of this.__menuItems){
			this.__nativeTray.AddMenuItem(item.id, item.caption);
		}
		this.__nativeTray.OnMenuItem((id) => {
			for(const cb of this.__itemCallbacks){
				cb(id);
			}
		})
		console.log("starting...");
		this.__nativeTray.Start(this.__iconPath);
		console.log("started");
	}
	item(cb) {
		if ("function" === typeof cb) {
			this.__itemCallbacks.push(cb);
		}
	}
	balloon(title, text, timeout = 5000) {
		return new Promise((resolve) => {
			this.__nativeTray.ShowBalloon(title, text, timeout, () => {
				resolve();
			})
		});
	}
	exit(){
		this.__nativeTray.Stop();
	}
}

module.exports = WindowsTryicon;
