// AudioManager.h
#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <irrKlang.h>
#include <string>
#include <map>
#include <iostream>

using namespace irrklang;

class AudioManager {
private:
    // Singleton
    static AudioManager* instance;

    // Motor de sonido irrKlang
    ISoundEngine* soundEngine;

    // Referencias a sonidos para control
    std::map<std::string, ISound*> sounds;

    // Volumen
    float musicVolume;
    float effectVolume;

    // Música de fondo actual
    std::string currentMusicName;

    // Constructor privado (patrón Singleton)
    AudioManager();

public:
    // Obtener instancia
    static AudioManager* getInstance();

    // Inicializar el sistema de audio
    bool init();

    // Cerrar y liberar recursos
    void close();

    // Cargar y reproducir música de fondo
    bool playMusic(const std::string& fileName, bool loop = true, float volume = 1.0f);

    // Reproducir efecto de sonido
    void playSound(const std::string& fileName, float volume = 1.0f);

    // Pausar toda la música
    void pauseMusic();

    // Reanudar toda la música
    void resumeMusic();

    // Detener toda la música
    void stopMusic();

    // Establecer volumen de la música (0.0-1.0)
    void setMusicVolume(float volume);

    // Establecer volumen de los efectos (0.0-1.0)
    void setEffectVolume(float volume);

    // Obtener motor de sonido
    ISoundEngine* getSoundEngine() { return soundEngine; }

    // Destructor
    ~AudioManager();
};

#endif // AUDIO_MANAGER_H