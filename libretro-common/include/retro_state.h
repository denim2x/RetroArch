/* Copyright  (C) 2021 denim2x
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this file (retro_state.h).
 * ---------------------------------------------------------------------------------------
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __RETRO_ASSERT_H
#define __RETRO_ASSERT_H

/**
 * content_push_state:
 * @path      : path of saved state that shall be written to.
 * @save_to_disk: If false, saves the state onto undo_load_buf.
 * Save a state from memory to disk.
 *
 * Returns: true if successful, false otherwise.
 **/
bool content_push_state(const char *path, bool save_to_disk, bool autosave)
{
   retro_ctx_size_info_t info;
   void *data  = NULL;
   size_t serial_size;

   core_serialize_size(&info);

   if (info.size == 0)
      return false;
   serial_size = info.size;

   if (!save_state_in_background)
   {
      data = content_get_serialized_data(&serial_size);

      if (!data)
      {
         RARCH_ERR("[State]: %s \"%s\".\n",
               msg_hash_to_str(MSG_FAILED_TO_SAVE_STATE_TO),
               path);
         return false;
      }

      RARCH_LOG("[State]: %s \"%s\", %u %s.\n",
            msg_hash_to_str(MSG_SAVING_STATE),
            path,
            (unsigned)serial_size,
            msg_hash_to_str(MSG_BYTES));
   }

   if (save_to_disk)
   {
      if (path_is_valid(path) && !autosave)
      {
         /* Before overwriting the savestate file, load it into a buffer
         to allow undo_save_state() to work */
         /* TODO/FIXME - Use msg_hash_to_str here */
         RARCH_LOG("[State]: %s ...\n",
               msg_hash_to_str(MSG_FILE_ALREADY_EXISTS_SAVING_TO_BACKUP_BUFFER));

         task_push_load_and_save_state(path, data, serial_size, true, autosave);
      }
      else
         task_push_save_state(path, data, serial_size, autosave);
   }
   else
   {
      if (!data)
         data = content_get_serialized_data(&serial_size);

      if (!data)
      {
         RARCH_ERR("[State]: %s \"%s\".\n",
               msg_hash_to_str(MSG_FAILED_TO_SAVE_STATE_TO),
               path);
         return false;
      }
      /* save_to_disk is false, which means we are saving the state
      in undo_load_buf to allow content_undo_load_state() to restore it */

      /* If we were holding onto an old state already, clean it up first */
      if (undo_load_buf.data)
      {
         free(undo_load_buf.data);
         undo_load_buf.data = NULL;
      }

      undo_load_buf.data = malloc(serial_size);
      if (!undo_load_buf.data)
      {
         free(data);
         return false;
      }

      memcpy(undo_load_buf.data, data, serial_size);
      free(data);
      undo_load_buf.size = serial_size;
      strlcpy(undo_load_buf.path, path, sizeof(undo_load_buf.path));
   }

   return true;
}

#endif
