/* ASEPRITE
 * Copyright (C) 2001-2012  David Capello
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef FILE_FILE_H_INCLUDED
#define FILE_FILE_H_INCLUDED

#include "base/shared_ptr.h"
#include "raster/pixel_format.h"

#include <stdio.h>
#include <vector>
#include <string>

#define FILE_LOAD_SEQUENCE_NONE         0x00000001
#define FILE_LOAD_SEQUENCE_ASK          0x00000002
#define FILE_LOAD_SEQUENCE_YES          0x00000004
#define FILE_LOAD_ONE_FRAME             0x00000008

class Document;
class Cel;
class Image;
class Layer;
class LayerImage;
class Mutex;
class Palette;

class FileFormat;
class FormatOptions;

// File operations.
typedef enum { FileOpLoad,
               FileOpSave } FileOpType;

// Structure to load & save files.
struct FileOp
{
  FileOpType type;              // Operation type: 0=load, 1=save.
  FileFormat* format;
  void* format_data;            // Custom data for the FileFormat::onLoad/onSave operations.
  Document* document;           // Loaded document, or document to be saved.
  std::string filename;         // File-name to load/save.

  // Shared fields between threads.
  Mutex* mutex;                 // Mutex to access to the next two fields.
  float progress;               // Progress (1.0 is ready).
  std::string error;            // Error string.
  bool done : 1;                // True if the operation finished.
  bool stop : 1;                // Force the break of the operation.
  bool oneframe : 1;            // Load just one frame (in formats
                                // that support animation like
                                // GIF/FLI/ASE).

  // Data for sequences.
  struct {
    std::vector<std::string> filename_list; // All file names to load/save.
    Palette* palette;           // Palette of the sequence.
    Image* image;               // Image to be saved/loaded.
    // For the progress bar.
    float progress_offset;      // Progress offset from the current frame.
    float progress_fraction;    // Progress fraction for one frame.
    // To load sequences.
    int frame;
    bool has_alpha;
    LayerImage* layer;
    Cel* last_cel;
    SharedPtr<FormatOptions> format_options;
  } seq;

  bool has_error() const {
    return !this->error.empty();
  }

  bool is_sequence() const {
    return !this->seq.filename_list.empty();
  }

};

// Available extensions for each load/save operation.

void get_readable_extensions(char* buf, int size);
void get_writable_extensions(char* buf, int size);

// High-level routines to load/save documents.

Document* load_document(const char* filename);
int save_document(Document* document);

// Low-level routines to load/save documents.

FileOp* fop_to_load_document(const char* filename, int flags);
FileOp* fop_to_save_document(Document* document);
void fop_operate(FileOp* fop);
void fop_done(FileOp* fop);
void fop_stop(FileOp* fop);
void fop_free(FileOp* fop);

// Does extra post-load processing which may require user intervention.
void fop_post_load(FileOp* fop);

void fop_sequence_set_format_options(FileOp* fop, const SharedPtr<FormatOptions>& format_options);
void fop_sequence_set_color(FileOp* fop, int index, int r, int g, int b);
void fop_sequence_get_color(FileOp* fop, int index, int *r, int *g, int *b);
Image* fop_sequence_image(FileOp* fi, PixelFormat pixelFormat, int w, int h);

void fop_error(FileOp* fop, const char *error, ...);
void fop_progress(FileOp* fop, float progress);

float fop_get_progress(FileOp* fop);
bool fop_is_done(FileOp* fop);
bool fop_is_stop(FileOp* fop);

int fgetw(FILE* file);
long fgetl(FILE* file);
int fputw(int w, FILE* file);
int fputl(long l, FILE* file);

#endif
