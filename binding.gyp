{
	"targets": [
		{
			"includes": [
				"auto.gypi"
			],
			"conditions": [
        		['OS=="win"', {
					"sources": [
						"TrayWrapper.cpp",
						"TrayIcon.cpp"
					],
					'msbuild_settings': {
						"ClCompile": {
							"RuntimeLibrary": "MultiThreaded"
						}
					}
				}]
			]
		}
	],
	"includes": [
		"auto-top.gypi"
	]
}
