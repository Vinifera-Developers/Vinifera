#pragma once

#include "always.h"
#include "movieplayback_backend.h"
#include "rect.h"


bool SDL_Movie_Present_Frame(const MovieVideoFrame &frame, const Rect &destination_rect);
bool SDL_Movie_Repaint();
bool SDL_Movie_Queue_Audio(const void *data, int data_length, int sample_rate, int channels, MovieSampleFormat format);
void SDL_Movie_Flush_Audio();
int SDL_Movie_Get_Queued_Audio_Size();
bool SDL_Movie_Has_Audio();
void SDL_Movie_Shutdown();
