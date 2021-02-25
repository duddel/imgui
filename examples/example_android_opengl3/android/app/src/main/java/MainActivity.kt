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
        val input_method_manager = getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
        input_method_manager.showSoftInput(this.window.decorView, 0)
    }

    fun hideSoftInput() {
        val input_method_manager = getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
        input_method_manager.hideSoftInputFromWindow(this.window.decorView.windowToken, 0)
    }

    // Queue for the Unicode characters to be polled from native code (via pollUnicodeChar())
    private var UnicodeCharacterQueue: LinkedBlockingQueue<Int> = LinkedBlockingQueue()

    // We assume dispatchKeyEvent() of the NativeActivity is actually called for every
    // KeyEvent and not consumed by any View before it reaches here
    override fun dispatchKeyEvent(event: KeyEvent): Boolean {
        var unicode_character = event.getUnicodeChar(event.metaState)

        if (event.action == KeyEvent.ACTION_DOWN) {
            if (unicode_character != 0) {
                UnicodeCharacterQueue.offer(Integer.valueOf(unicode_character))
            } else {
                UnicodeCharacterQueue.offer(Integer.valueOf(0))
            }
        } else if (event.action == KeyEvent.ACTION_MULTIPLE) {
            unicode_character = Character.codePointAt(event.characters, 0)
            UnicodeCharacterQueue.offer(Integer.valueOf(unicode_character))
        }

        return super.dispatchKeyEvent(event)
    }

    fun pollUnicodeChar(): Int {
        return if (!UnicodeCharacterQueue.isEmpty()) UnicodeCharacterQueue.poll().toInt() else 0
    }
}
