
#ifndef ARX_IO_CINEMATICLOAD_H
#define ARX_IO_CINEMATICLOAD_H

namespace fs {
class path;
}

class Cinematic;

bool loadCinematic(Cinematic * c, const fs::path & file);

#endif // ARX_IO_CINEMATICLOAD_H
