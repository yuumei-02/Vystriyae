// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#include <mcu/core.h>
#include <mcu/containers.h>

#include "core.h"
#include "ime.h"

// @Todo: turn the IME into a callback that can be set via the API
void ime_transform(Vector* InputEvent_queue) {
   static bool ime_on = false;

   foreach ((*InputEvent_queue), i) {
      InputEvent* event = Vector_get(InputEvent_queue, i);

      if (event->type == IET_Key) {
         /* if (event->key.code == KC_t) ime_on = !ime_on; */
         continue;
      }

      if (event->type != IET_Text || ime_on == false) continue;

      switch (event->text.code_point) {
         case 'q':  event->text.code_point = u'й'; break;
         case 'w':  event->text.code_point = u'ц'; break;
         case 'e':  event->text.code_point = u'у'; break;
         case 'r':  event->text.code_point = u'к'; break;
         case 't':  event->text.code_point = u'е'; break;
         case 'y':  event->text.code_point = u'н'; break;
         case 'u':  event->text.code_point = u'г'; break;
         case 'i':  event->text.code_point = u'ш'; break;
         case 'o':  event->text.code_point = u'щ'; break;
         case 'p':  event->text.code_point = u'з'; break;
         case '[':  event->text.code_point = u'х'; break;
         case ']':  event->text.code_point = u'ъ'; break;
         case 'a':  event->text.code_point = u'ф'; break;
         case 's':  event->text.code_point = u'ы'; break;
         case 'd':  event->text.code_point = u'в'; break;
         case 'f':  event->text.code_point = u'а'; break;
         case 'g':  event->text.code_point = u'п'; break;
         case 'h':  event->text.code_point = u'р'; break;
         case 'j':  event->text.code_point = u'о'; break;
         case 'k':  event->text.code_point = u'л'; break;
         case 'l':  event->text.code_point = u'д'; break;
         case ';':  event->text.code_point = u'ж'; break;
         case '\'': event->text.code_point = u'э'; break;
         case 'z':  event->text.code_point = u'я'; break;
         case 'x':  event->text.code_point = u'ч'; break;
         case 'c':  event->text.code_point = u'с'; break;
         case 'v':  event->text.code_point = u'м'; break;
         case 'b':  event->text.code_point = u'и'; break;
         case 'n':  event->text.code_point = u'т'; break;
         case 'm':  event->text.code_point = u'ь'; break;
         case ',':  event->text.code_point = u'б'; break;
         case '.':  event->text.code_point = u'ю'; break;
         case '/':  event->text.code_point = u'.'; break;
      
         default: break;
      }
   }
}

