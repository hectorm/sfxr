#ifndef TOOLS_H
#define TOOLS_H

int LoadTGA(Spriteset &tiles, const char *filename) {
  FILE *file;
  unsigned char byte, crap[16], id_length;
  int n, width, height, /* channels, */ x, y;
  file = fopen(filename, "rb");
  if (!file)
    return -1;
  if (fread(&id_length, 1, 1, file) != 1)
    return -1;
  if (fread(crap, 1, 11, file) != 11)
    return -1;
  width = 0;
  height = 0;
  if (fread(&width, 1, 2, file) != 2) // width
    return -1;
  if (fread(&height, 1, 2, file) != 2) // height
    return -1;
  if (fread(&byte, 1, 1, file) != 1) // bits
    return -1;
  // channels = byte / 8;
  if (fread(&byte, 1, 1, file) != 1) // image descriptor byte (per-bit info)
    return -1;
  for (n = 0; n < id_length; n++)
    if (fread(&byte, 1, 1, file) != 1) // image description
      return -1;
  tiles.data = (DWORD *)malloc(width * height * sizeof(DWORD));
  for (y = height - 1; y >= 0; y--)
    for (x = 0; x < width; x++) {
      DWORD pixel = 0;
      if (fread(&byte, 1, 1, file) != 1)
        return -1;
      pixel |= byte;
      if (fread(&byte, 1, 1, file) != 1)
        return -1;
      pixel |= byte << 8;
      if (fread(&byte, 1, 1, file) != 1)
        return -1;
      pixel |= byte << 16;
      tiles.data[y * width + x] = pixel;
    }
  if (fclose(file) != 0)
    return -1;
  tiles.height = height;
  tiles.width = height;
  tiles.pitch = width;

  return 0;
}

void ClearScreen(DWORD color) {
  for (int y = 0; y < 480; y++) {
    int offset = y * ddkpitch;
    for (int x = 0; x < 640; x += 8) {
      ddkscreen32[offset++] = color;
      ddkscreen32[offset++] = color;
      ddkscreen32[offset++] = color;
      ddkscreen32[offset++] = color;
      ddkscreen32[offset++] = color;
      ddkscreen32[offset++] = color;
      ddkscreen32[offset++] = color;
      ddkscreen32[offset++] = color;
    }
  }
}

void DrawBar(int sx, int sy, int w, int h, DWORD color) {
  for (int y = sy; y < sy + h; y++) {
    int offset = y * ddkpitch + sx;
    int x1 = 0;
    if (w > 8)
      for (x1 = 0; x1 < w - 8; x1 += 8) {
        ddkscreen32[offset++] = color;
        ddkscreen32[offset++] = color;
        ddkscreen32[offset++] = color;
        ddkscreen32[offset++] = color;
        ddkscreen32[offset++] = color;
        ddkscreen32[offset++] = color;
        ddkscreen32[offset++] = color;
        ddkscreen32[offset++] = color;
      }
    for (int x = x1; x < w; x++)
      ddkscreen32[offset++] = color;
  }
}

void DrawBox(int sx, int sy, int w, int h, DWORD color) {
  DrawBar(sx, sy, w, 1, color);
  DrawBar(sx, sy, 1, h, color);
  DrawBar(sx + w, sy, 1, h, color);
  DrawBar(sx, sy + h, w + 1, 1, color);
}

void DrawSprite(Spriteset &sprites, int sx, int sy, int i, DWORD color) {
  for (int y = 0; y < sprites.height; y++) {
    int offset = (sy + y) * ddkpitch + sx;
    int spoffset = y * sprites.pitch + i * sprites.width;
    if (color & 0xFF000000)
      for (int x = 0; x < sprites.width; x++) {
        DWORD p = sprites.data[spoffset++];
        if (p != 0x300030)
          ddkscreen32[offset + x] = p;
      }
    else
      for (int x = 0; x < sprites.width; x++) {
        DWORD p = sprites.data[spoffset++];
        if (p != 0x300030)
          ddkscreen32[offset + x] = color;
      }
  }
}

void DrawText(int sx, int sy, DWORD color, const char *string, ...) {
  char string2[256];
  va_list args;

  va_start(args, string);
  vsprintf(string2, string, args);
  va_end(args);

  int len = strlen(string2);
  for (int i = 0; i < len; i++)
    DrawSprite(font, sx + i * 8, sy, string2[i] - ' ', color);
}

bool MouseInBox(int x, int y, int w, int h) {
  if (mouse_x >= x && mouse_x < x + w && mouse_y >= y && mouse_y < y + h)
    return true;
  return false;
}

#endif
