import os
import threading
from ctypes import CDLL, c_char_p
from tkinterdnd2 import TkinterDnD, DND_FILES
import tkinter as tk
from tkinter import filedialog, messagebox, scrolledtext
import sys

# === Load the DLL ===
def get_dll_path():
    if getattr(sys, 'frozen', False):  # PyInstaller mode
        base_path = sys._MEIPASS
    else:
        base_path = os.path.dirname(os.path.abspath(__file__))
    return os.path.join(base_path, "video_decoder.dll")

dll_path = get_dll_path()

if not os.path.exists(dll_path):
    raise FileNotFoundError(f"video_decoder.dll not found at: {dll_path}")

decoder = CDLL(dll_path)

decoder.decode_audio.argtypes = [c_char_p]
decoder.decode_audio.restype = c_char_p

decoder.decode_qr.argtypes = [c_char_p]
decoder.decode_qr.restype = c_char_p

def decode_audio_from_video(filepath):
    return decoder.decode_audio(filepath.encode()).decode()

def extract_qr_from_video(filepath):
    return decoder.decode_qr(filepath.encode()).decode()

# === GUI Class ===
class VideoDecoderApp:
    def __init__(self, master):
        self.master = master
        master.title("Video Decoder (QR + Audio Stego)")

        self.label = tk.Label(master, text="Drag and drop or select a video file:")
        self.label.pack(pady=10)

        self.select_button = tk.Button(master, text="Select Video File", command=self.select_file)
        self.select_button.pack(pady=5)

        self.output_text = scrolledtext.ScrolledText(master, height=20, width=80)
        self.output_text.pack(padx=10, pady=10)

        self.master.drop_target_register(DND_FILES)
        self.master.dnd_bind('<<Drop>>', self.drop_event)

    def select_file(self):
        file_path = filedialog.askopenfilename(filetypes=[("Video files", "*.mp4 *.mov *.avi *.mkv")])
        if file_path:
            self.process_file(file_path)

    def drop_event(self, event):
        file_path = event.data.strip('{}')  # Handles spaces
        if os.path.isfile(file_path):
            self.process_file(file_path)
        else:
            messagebox.showerror("Error", "Invalid file dropped.")

    def process_file(self, file_path):
        self.output_text.delete(1.0, tk.END)
        self.output_text.insert(tk.END, f"Processing file: {file_path}\n\n")
        threading.Thread(target=self.decode_thread, args=(file_path,)).start()

    def decode_thread(self, video_path):
        try:
            audio_data = decode_audio_from_video(video_path)
            qr_data = extract_qr_from_video(video_path)

            if audio_data == qr_data and audio_data and qr_data:
                self.output_text.insert(tk.END, "[*] Decoding data from QR and audio ...\n")
                self.output_text.insert(tk.END, split_and_return_data(qr_data) + "\n")
            else:
                self.output_text.insert(tk.END, "[*] Decoding from audio...\n")
                if audio_data:
                    self.output_text.insert(tk.END, split_and_return_data(audio_data) + "\n\n")
                else:
                    self.output_text.insert(tk.END, "[-] audio not found.\n")
                    
                self.output_text.insert(tk.END, "[*] Decoding from QR code...\n")
                if qr_data:
                    self.output_text.insert(tk.END, split_and_return_data(qr_data) + "\n")
                else:
                    self.output_text.insert(tk.END, "[-] QR code not found.\n")

        except Exception as e:
            self.output_text.insert(tk.END, f"[!] Error during processing: {e}\n")

# === Helper to parse decoded data ===
def split_and_return_data(extracted_message):
    data = ""
    try:
        parts = extracted_message.split(',')
        tablet_serial_number = parts[0] if len(parts) > 0 and parts[0] else "missing tablet serial number!!\n"
        if len(tablet_serial_number) < 10:
            data+= f"tablet serial number might be croppet!! printing the exist message: {tablet_serial_number}\n"
        elif len(tablet_serial_number) > 10:
            data+= f"invalid tablet serial number decoded!! the decoded text is: {tablet_serial_number}\n"
        else:
            data += f"Tablet SN: {tablet_serial_number}\n"
        usb_serial_number = parts[1] if len(parts) > 1 and parts[1] else "missing usb serial number!!\n"
        if len(usb_serial_number) < 12 and usb_serial_number != "missing usb serial number!!\n":
            data += f"usb serial number might be croppet!! printing the exist message: {usb_serial_number}\n"
        elif len(usb_serial_number) > 12:
            data += f"invalid usb serial number decoded!! the decoded text is: {usb_serial_number}\n"
        else:
            data += f"USB SN: {usb_serial_number}\n"
        date = parts[2] if len(parts) > 2 else "missing date"
        if len(date) < 19 and date != "missing date":
            data += f"time signature might be croppet!! printing the exist message: {date}\n"
        elif len(date) > 19:
            data += f"invalid time signature decoded!! the decoded text is: {date}\n"
        else:
            data+=f"Time: {date}\n"
        #return f"Tablet SN: {tablet_serial_number}\nUSB SN: {usb_serial_number}\nTime: {date}"
        return data
    except Exception as e:
        return f"[!] Error decoding structure: {e}\n"

# === Run GUI ===
if __name__ == "__main__":
    root = TkinterDnD.Tk()
    app = VideoDecoderApp(root)
    root.mainloop()
