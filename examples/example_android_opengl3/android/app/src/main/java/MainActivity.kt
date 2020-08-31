package imgui.example.android

import android.app.NativeActivity
import android.os.Bundle
import android.content.Context
import android.view.inputmethod.InputMethodManager

class MainActivity : NativeActivity() {
    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
    }

    fun showSoftInput() {
        val inputMM = getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
        inputMM.showSoftInput(this.window.decorView, 0)
    }

    fun hideSoftInput() {
        val inputMM = getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
        inputMM.hideSoftInputFromWindow(this.window.decorView.windowToken, 0)
    }
}
