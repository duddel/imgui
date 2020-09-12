package imgui.example.android

import android.app.NativeActivity
import android.os.Bundle
import android.content.Context
import android.view.inputmethod.InputMethodManager
import android.view.KeyEvent
import java.util.concurrent.LinkedBlockingQueue

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

    // Queue for the Unicode characters to be polled from native code (via pollUnicodeChar())
    private var unicCharQueue: LinkedBlockingQueue<Int> = LinkedBlockingQueue()

    // We assume dispatchKeyEvent() of the NativeActivity is actually called for every
    // KeyEvent and not consumed by any View before it reaches here
    override fun dispatchKeyEvent(event: KeyEvent): Boolean {
        var unic = event.getUnicodeChar(event.metaState)

        if (event.action == KeyEvent.ACTION_DOWN) {
            if (unic != 0) {
                unicCharQueue.offer(Integer.valueOf(unic))
            } else {
                unicCharQueue.offer(Integer.valueOf(0))
            }
        } else if (event.action == KeyEvent.ACTION_MULTIPLE) {
            unic = Character.codePointAt(event.characters, 0)
            unicCharQueue.offer(Integer.valueOf(unic))
        }

        return super.dispatchKeyEvent(event)
    }

    fun pollUnicodeChar(): Int {
        return if (!unicCharQueue.isEmpty()) unicCharQueue.poll().toInt() else 0
    }
}
