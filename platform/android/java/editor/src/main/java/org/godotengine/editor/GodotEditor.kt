/**************************************************************************/
/*  GodotEditor.kt                                                        */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

package org.godotengine.editor

import android.Manifest
import android.app.ActivityManager
import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.*
import android.util.Log
import android.widget.Toast
import androidx.annotation.CallSuper
import androidx.window.layout.WindowMetricsCalculator
import org.godotengine.godot.GodotActivity
import org.godotengine.godot.GodotLib
import org.godotengine.godot.utils.PermissionsUtil
import org.godotengine.godot.utils.ProcessPhoenix
import java.util.*
import kotlin.math.min

/**
 * Base class for the Godot Android Editor activities.
 *
 * This provides the basic templates for the activities making up this application.
 * Each derived activity runs in its own process, which enable up to have several instances of
 * the Godot engine up and running at the same time.
 *
 * It also plays the role of the primary editor window.
 */
open class GodotEditor : GodotActivity() {

	companion object {
		private val TAG = GodotEditor::class.java.simpleName

		private const val WAIT_FOR_DEBUGGER = false

		private const val EXTRA_COMMAND_LINE_PARAMS = "command_line_params"

		// Command line arguments
		private const val EDITOR_ARG = "--editor"
		private const val EDITOR_ARG_SHORT = "-e"

		private const val PROJECT_MANAGER_ARG = "--project-manager"
		private const val PROJECT_MANAGER_ARG_SHORT = "-p"
		private const val PROJECT_MANAGER_PROCESS_NAME_SUFFIX = ":GodotProjectManager"

		/**
		 * Sets of constants to specify the window to use to run the project.
		 *
		 * Should match the values in 'editor/editor_settings.cpp' for the
		 * 'run/window_placement/android_window' setting.
		 */
		private const val ANDROID_WINDOW_AUTO = 0
		private const val ANDROID_WINDOW_SAME_AS_EDITOR = 1
		private const val ANDROID_WINDOW_SIDE_BY_SIDE_WITH_EDITOR = 2

		internal const val XR_MODE_ARG = "--xr-mode"

		// Info for the various classes used by the editor
		private val EDITOR_MAIN_INFO =
			EditorInstanceInfo(GodotEditor::class.java, 777, ":GodotEditor")
		internal val PROJECT_MANAGER_INFO =
			EditorInstanceInfo(GodotProjectManager::class.java, 555, ":GodotProjectManager")
		// The classes referenced below are only available on openxr builds of the editor.
		private val XR_EDITOR_MAIN_INFO =
				EditorInstanceInfo("org.godotengine.editor.GodotXREditor", 1777, ":GodotXREditor")
		private val XR_PROJECT_MANAGER_INFO =
				EditorInstanceInfo("org.godotengine.editor.GodotXRProjectManager", 1555, ":GodotXRProjectManager")
		private val XR_RUN_GAME_INFO =
				EditorInstanceInfo("org.godotengine.editor.GodotXRGame", 1667, ":GodotXRGame")
	}

	private val runGameInfo: EditorInstanceInfo by lazy {
		EditorInstanceInfo(
			GodotGame::class.java,
			667,
			":GodotGame",
			Build.VERSION.SDK_INT >= Build.VERSION_CODES.N && (isInMultiWindowMode || isLargeScreen)
		)
	}

	private val commandLineParams = ArrayList<String>()

	override fun onCreate(savedInstanceState: Bundle?) {
		// We exclude certain permissions from the set we request at startup, as they'll be
		// requested on demand based on use-cases.
		PermissionsUtil.requestManifestPermissions(this, setOf(Manifest.permission.RECORD_AUDIO))

		val params = intent.getStringArrayExtra(COMMAND_LINE_PARAMS)
		updateCommandLineParams(params?.asList() ?: emptyList())

		if (BuildConfig.BUILD_TYPE == "dev" && WAIT_FOR_DEBUGGER) {
			Debug.waitForDebugger()
		}

		super.onCreate(savedInstanceState)
	}

	override fun onGodotSetupCompleted() {
		super.onGodotSetupCompleted()
		val longPressEnabled = enableLongPressGestures()
		val panScaleEnabled = enablePanAndScaleGestures()

		checkForProjectPermissionsToEnable()

		runOnUiThread {
			// Enable long press, panning and scaling gestures
			godotFragment?.godot?.renderView?.inputHandler?.apply {
				enableLongPress(longPressEnabled)
				enablePanningAndScalingGestures(panScaleEnabled)
			}
		}
	}

	/**
	 * Check for project permissions to enable
	 */
	protected open fun checkForProjectPermissionsToEnable() {
		// Check for RECORD_AUDIO permission
		val audioInputEnabled = java.lang.Boolean.parseBoolean(GodotLib.getGlobal("audio/driver/enable_input"));
		if (audioInputEnabled) {
			PermissionsUtil.requestPermission(Manifest.permission.RECORD_AUDIO, this)
		}
	}

	@CallSuper
	protected open fun updateCommandLineParams(args: List<String>) {
		// Update the list of command line params with the new args
		commandLineParams.clear()
		if (args.isNotEmpty()) {
			commandLineParams.addAll(args)
		}
		if (BuildConfig.BUILD_TYPE == "dev") {
			commandLineParams.add("--benchmark")
		}
	}

	final override fun getCommandLine() = commandLineParams

	private fun isXrAvailable() = BuildConfig.XR_MODE

	private fun getEditorInstanceInfo(args: Array<String>): EditorInstanceInfo {
		var hasEditor = false
		var hasProjectManager = false
		var launchInXr = false
		var i = 0
		while (i < args.size) {
			when (args[i++]) {
				EDITOR_ARG, EDITOR_ARG_SHORT -> hasEditor = true
				PROJECT_MANAGER_ARG, PROJECT_MANAGER_ARG_SHORT -> hasProjectManager = true
				XR_MODE_ARG -> {
					val argValue = args[i++]
					launchInXr = isXrAvailable() && ("on" == argValue)
				}
			}
		}

		return if (hasEditor) {
			if (launchInXr) {
				XR_EDITOR_MAIN_INFO
			} else {
				EDITOR_MAIN_INFO
			}
		} else if (hasProjectManager) {
			if (launchInXr) {
				XR_PROJECT_MANAGER_INFO
			} else {
				PROJECT_MANAGER_INFO
			}
		} else {
			if (launchInXr) {
				XR_RUN_GAME_INFO
			} else {
				runGameInfo
			}
		}
	}

	override fun onNewGodotInstanceRequested(args: Array<String>): Int {
		// Parse the arguments to figure out which activity to start.
		var targetClass: Class<*> = GodotGame::class.java
		var instanceId = GAME_ID

		// Whether we should launch the new godot instance in an adjacent window
		// https://developer.android.com/reference/android/content/Intent#FLAG_ACTIVITY_LAUNCH_ADJACENT
		var launchAdjacent = shouldGameLaunchAdjacent()

		for (arg in args) {
			if (EDITOR_ARG == arg || EDITOR_ARG_SHORT == arg) {
				targetClass = GodotEditor::class.java
				launchAdjacent = false
				instanceId = EDITOR_ID
				break
			}

			if (PROJECT_MANAGER_ARG == arg || PROJECT_MANAGER_ARG_SHORT == arg) {
				targetClass = GodotProjectManager::class.java
				launchAdjacent = false
				instanceId = PROJECT_MANAGER_ID
				break
			}
		}

		// Launch a new activity
		val newInstance = Intent(this, targetClass)
			.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
			.putExtra(EXTRA_COMMAND_LINE_PARAMS, args)
		if (launchAdjacent) {
			newInstance.addFlags(Intent.FLAG_ACTIVITY_LAUNCH_ADJACENT)
		}
		if (targetClass == javaClass) {
			Log.d(TAG, "Restarting $targetClass with parameters ${args.contentToString()}")
			ProcessPhoenix.triggerRebirth(this, newInstance)
		} else {
			Log.d(TAG, "Starting $targetClass with parameters ${args.contentToString()}")
			newInstance.putExtra(EXTRA_NEW_LAUNCH, true)
			startActivity(newInstance)
		}
		return instanceId
	}

/* TODO - FROM XR EDITOR - don't know how to combine this with the one added in master

	final override fun onNewGodotInstanceRequested(args: Array<String>): Int {
		val editorInstanceInfo = getEditorInstanceInfo(args)

		// Launch a new activity
		val newInstance = Intent()
			.setComponent(ComponentName(this, editorInstanceInfo.instanceClassName))
			.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
			.putExtra(COMMAND_LINE_PARAMS, args)
		if (editorInstanceInfo.launchAdjacent && Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
			newInstance.addFlags(Intent.FLAG_ACTIVITY_LAUNCH_ADJACENT)
		}
		if (editorInstanceInfo.instanceClassName == javaClass.name) {
			Log.d(TAG, "Restarting ${editorInstanceInfo.instanceClassName}")
			ProcessPhoenix.triggerRebirth(this, newInstance)
		} else {
			Log.d(TAG, "Starting ${editorInstanceInfo.instanceClassName}")
			startActivity(newInstance)
		}
		return editorInstanceInfo.instanceId
	}

*/

	override fun onGodotForceQuit(godotInstanceId: Int): Boolean {
		val targetClass: Class<*>?
		val processNameSuffix: String
		when (godotInstanceId) {
			GAME_ID -> {
				processNameSuffix = GAME_PROCESS_NAME_SUFFIX
				targetClass = GodotGame::class.java
			}
			EDITOR_ID -> {
				processNameSuffix = EDITOR_PROCESS_NAME_SUFFIX
				targetClass = GodotEditor::class.java
			}
			PROJECT_MANAGER_ID -> {
				processNameSuffix = PROJECT_MANAGER_PROCESS_NAME_SUFFIX
				targetClass = GodotProjectManager::class.java
			}
			else -> {
				processNameSuffix = ""
				targetClass = null
			}
		}

		if (targetClass == javaClass) {
			Log.d(TAG, "Force quitting $targetClass")
			ProcessPhoenix.forceQuit(this)
			return true
		}

		if (processNameSuffix.isBlank()) {
			return false
		}

		val activityManager = getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
		val runningProcesses = activityManager.runningAppProcesses
		for (runningProcess in runningProcesses) {
			if (runningProcess.processName.endsWith(processNameSuffix)) {
				if (targetClass == null) {
					// Killing process directly
					Log.v(TAG, "Killing Godot process ${runningProcess.processName}")
					Process.killProcess(runningProcess.pid)
				} else {
					// Activity is running; sending a request for self termination.
					Log.v(TAG, "Sending force quit request to $targetClass running on process ${runningProcess.processName}")
					val forceQuitIntent = Intent(this, targetClass).putExtra(EXTRA_FORCE_QUIT, true)
					startActivity(forceQuitIntent)
				}
				return true
			}
		}
		return super.onGodotForceQuit(godotInstanceId)
	}

/* TODO - FROM XR EDITOR - don't know how to combine this with the one added in master

	final override fun onGodotForceQuit(godotInstanceId: Int): Boolean {
		val processNameSuffix = getProcessNameForInstanceId(godotInstanceId)
		if (processNameSuffix.isNotBlank()) {
			val activityManager = getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
			val runningProcesses = activityManager.runningAppProcesses
			for (runningProcess in runningProcesses) {
				if (runningProcess.processName.endsWith(processNameSuffix)) {
					Log.v(TAG, "Killing Godot process ${runningProcess.processName}")
					Process.killProcess(runningProcess.pid)
					return true
				}
			}
		}
		return super.onGodotForceQuit(godotInstanceId)
	}

*/

	private fun getProcessNameForInstanceId(instanceId: Int): String {
		return when (instanceId) {
			runGameInfo.instanceId -> runGameInfo.processNameSuffix
			EDITOR_MAIN_INFO.instanceId -> EDITOR_MAIN_INFO.processNameSuffix
			PROJECT_MANAGER_INFO.instanceId -> PROJECT_MANAGER_INFO.processNameSuffix

			XR_RUN_GAME_INFO.instanceId -> XR_RUN_GAME_INFO.processNameSuffix
			XR_EDITOR_MAIN_INFO.instanceId -> XR_EDITOR_MAIN_INFO.processNameSuffix
			XR_PROJECT_MANAGER_INFO.instanceId -> XR_PROJECT_MANAGER_INFO.processNameSuffix

			else -> ""
		}
	}

	// Get the screen's density scale
	private val isLargeScreen: Boolean
		// Get the minimum window size // Correspond to the EXPANDED window size class.
		get() {
			val metrics = WindowMetricsCalculator.getOrCreate().computeMaximumWindowMetrics(this)

			// Get the screen's density scale
			val scale = resources.displayMetrics.density

			// Get the minimum window size
			val minSize = min(metrics.bounds.width(), metrics.bounds.height()).toFloat()
			val minSizeDp = minSize / scale
			return minSizeDp >= 840f // Correspond to the EXPANDED window size class.
		}

	override fun setRequestedOrientation(requestedOrientation: Int) {
		if (!overrideOrientationRequest()) {
			super.setRequestedOrientation(requestedOrientation)
		}
	}

	/**
	 * The Godot Android Editor sets its own orientation via its AndroidManifest
	 */
	protected open fun overrideOrientationRequest() = true

	/**
	 * Enable long press gestures for the Godot Android editor.
	 */
	protected open fun enableLongPressGestures() =
		java.lang.Boolean.parseBoolean(GodotLib.getEditorSetting("interface/touchscreen/enable_long_press_as_right_click"))

	/**
	 * Enable pan and scale gestures for the Godot Android editor.
	 */
	protected open fun enablePanAndScaleGestures() =
		java.lang.Boolean.parseBoolean(GodotLib.getEditorSetting("interface/touchscreen/enable_pan_and_scale_gestures"))

	private fun shouldGameLaunchAdjacent(): Boolean {
		return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
			try {
				when (Integer.parseInt(GodotLib.getEditorSetting("run/window_placement/android_window"))) {
					ANDROID_WINDOW_SAME_AS_EDITOR -> false
					ANDROID_WINDOW_SIDE_BY_SIDE_WITH_EDITOR -> true
					else -> {
						// ANDROID_WINDOW_AUTO
						isInMultiWindowMode || isLargeScreen
					}
				}
			} catch (e: NumberFormatException) {
				// Fall-back to the 'Auto' behavior
				isInMultiWindowMode || isLargeScreen
			}
		} else {
			false
		}
	}

	override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
		super.onActivityResult(requestCode, resultCode, data)
		// Check if we got the MANAGE_EXTERNAL_STORAGE permission
		if (requestCode == PermissionsUtil.REQUEST_MANAGE_EXTERNAL_STORAGE_REQ_CODE) {
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
				if (!Environment.isExternalStorageManager()) {
					Toast.makeText(
						this,
						R.string.denied_storage_permission_error_msg,
						Toast.LENGTH_LONG
					).show()
				}
			}
		}
	}

	override fun onRequestPermissionsResult(
		requestCode: Int,
		permissions: Array<String>,
		grantResults: IntArray
	) {
		super.onRequestPermissionsResult(requestCode, permissions, grantResults)
		// Check if we got access to the necessary storage permissions
		if (requestCode == PermissionsUtil.REQUEST_ALL_PERMISSION_REQ_CODE) {
			if (Build.VERSION.SDK_INT < Build.VERSION_CODES.R) {
				var hasReadAccess = false
				var hasWriteAccess = false
				for (i in permissions.indices) {
					if (Manifest.permission.READ_EXTERNAL_STORAGE == permissions[i] && grantResults[i] == PackageManager.PERMISSION_GRANTED) {
						hasReadAccess = true
					}
					if (Manifest.permission.WRITE_EXTERNAL_STORAGE == permissions[i] && grantResults[i] == PackageManager.PERMISSION_GRANTED) {
						hasWriteAccess = true
					}
				}
				if (!hasReadAccess || !hasWriteAccess) {
					Toast.makeText(
						this,
						R.string.denied_storage_permission_error_msg,
						Toast.LENGTH_LONG
					).show()
				}
			}
		}
	}
}
