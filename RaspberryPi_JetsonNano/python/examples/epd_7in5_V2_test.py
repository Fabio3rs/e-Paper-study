#!/usr/bin/python
# -*- coding:utf-8 -*-
import sys
import os
picdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'pic')
libdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'lib')
if os.path.exists(libdir):
    sys.path.append(libdir)

import logging
from waveshare_epd import epd7in5_V2
import time
from PIL import Image,ImageDraw,ImageFont
import traceback
import textwrap
from typing import List
import curses

from langchain.document_loaders import (
    CSVLoader,
    EverNoteLoader,
    PDFMinerLoader,
    TextLoader,
    UnstructuredEmailLoader,
    UnstructuredEPubLoader,
    UnstructuredHTMLLoader,
    UnstructuredMarkdownLoader,
    UnstructuredODTLoader,
    UnstructuredPowerPointLoader,
    UnstructuredWordDocumentLoader,
)

#from langchain.text_splitter import RecursiveCharacterTextSplitter
#from langchain.vectorstores import Chroma
#from langchain.embeddings import HuggingFaceEmbeddings
from langchain.docstore.document import Document
# logging.basicConfig(level=logging.DEBUG)

# Custom document loaders
class MyElmLoader(UnstructuredEmailLoader):
    """Wrapper to fallback to text/plain when default does not work"""

    def load(self) -> List[Document]:
        """Wrapper adding fallback for elm without html"""
        try:
            try:
                doc = UnstructuredEmailLoader.load(self)
            except ValueError as e:
                if 'text/html content not found in email' in str(e):
                    # Try plain text
                    self.unstructured_kwargs["content_source"]="text/plain"
                    doc = UnstructuredEmailLoader.load(self)
                else:
                    raise
        except Exception as e:
            # Add file_path to exception message
            raise type(e)(f"{self.file_path}: {e}") from e

        return doc

LOADER_MAPPING = {
    ".csv": (CSVLoader, {}),
    # ".docx": (Docx2txtLoader, {}),
    ".doc": (UnstructuredWordDocumentLoader, {}),
    ".docx": (UnstructuredWordDocumentLoader, {}),
    ".enex": (EverNoteLoader, {}),
    ".eml": (MyElmLoader, {}),
    ".epub": (UnstructuredEPubLoader, {}),
    ".html": (UnstructuredHTMLLoader, {}),
    ".md": (UnstructuredMarkdownLoader, {}),
    ".odt": (UnstructuredODTLoader, {}),
    ".pdf": (PDFMinerLoader, {}),
    ".ppt": (UnstructuredPowerPointLoader, {}),
    ".pptx": (UnstructuredPowerPointLoader, {}),
    ".txt": (TextLoader, {"encoding": "utf8"}),
    # Add more mappings for other file extensions and loaders as needed
}

def load_single_document(file_path: str) -> Document:
    ext = "." + file_path.rsplit(".", 1)[-1]
    if ext in LOADER_MAPPING:
        loader_class, loader_args = LOADER_MAPPING[ext]
        loader = loader_class(file_path, **loader_args)
        return loader.load()[0]

    raise ValueError(f"Unsupported file extension '{ext}'")

content = load_single_document("aen1.html")

# Function to draw text with automatic line wrapping
def draw_text_with_wrapping(draw, text_lines, position, font, fill):
    max_width = 800  # Adjust as needed
    margin = 5

    wrapped_lines = []
    for line in text_lines:
        wrapped_lines.extend(textwrap.wrap(line, width=60))  # Adjust width as needed

    # Draw the wrapped text
    draw.multiline_text(position, '\n'.join(wrapped_lines), font=font, fill=fill, spacing=4)

def main(stdscr):
    # Set up the screen
    curses.curs_set(0)  # Hide the cursor
    stdscr.clear()
    stdscr.refresh()

    # Enable keypad input
    stdscr.keypad(True)

    logging.info("epd7in5_V2 Demo")
    epd = epd7in5_V2.EPD()
            
    logging.info("init and Clear")
    epd.init()
    epd.Clear()

    font24 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), 24)
    font18 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), 18)

    font20 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), 20)


    time.sleep(2)

    # Drawing on the Vertical image
    logging.info("2.Drawing on the Vertical image...")
            
    # Example of drawing large text with automatic line wrapping
    long_text = content.page_content.splitlines()
    current_line = 0
    
    jump_lines = 40
    update = True

    # Main loop
    while True:
        if update:
            Limage = Image.new('1', (epd.height, epd.width), 255)  # 255: clear the frame
            draw = ImageDraw.Draw(Limage)
            draw_text_with_wrapping(draw, long_text[current_line::], (10, 0), font20, fill=0)
            epd.display(epd.getbuffer(Limage))
            update = False

        key = stdscr.getch()

        try:
            # Check for key up
            if key == curses.KEY_UP:
                stdscr.addstr(0, 0, "Key Up Pressed")
                current_line = min(current_line + jump_lines, len(long_text) - 1)
                update = True
            # Check for key down
            elif key == curses.KEY_DOWN:
                stdscr.addstr(0, 0, "Key Down Pressed")
                current_line = max(current_line - jump_lines, 0)
                update = True
            # Check for the 'q' key to quit
            elif key == ord('q'):
                break
            
        except IOError as e:
            logging.info(e)
            
        except KeyboardInterrupt:    
            logging.info("ctrl + c:")
            epd7in5_V2.epdconfig.module_exit()
            exit()

        stdscr.refresh()

    logging.info("Clear...")
    epd.init()
    epd.Clear()

    logging.info("Goto Sleep...")
    epd.sleep()

# Run the program
curses.wrapper(main)


