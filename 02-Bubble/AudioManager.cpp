// AudioManager.cpp
#include "AudioManager.h"

// Inicializar instancia singleton
AudioManager* AudioManager::instance = nullptr;

AudioManager::AudioManager()
    : soundEngine(nullptr), musicVolume(1.0f), effectVolume(1.0f), currentMusicName("") {
}

AudioManager* AudioManager::getInstance() {
    if (instance == nullptr) {
        instance = new AudioManager();
    }
    return instance;
}

bool AudioManager::init() {
    // Crear motor de sonido
    soundEngine = createIrrKlangDevice();

    if (!soundEngine) {
        std::cerr << "No se pudo inicializar el motor de audio irrKlang!" << std::endl;
        return false;
    }

    return true;
}

void AudioManager::close() {
    // Detener todos los sonidos
    if (soundEngine) {
        soundEngine->stopAllSounds();

        // Liberar recursos de los sonidos guardados
        for (auto& sound : sounds) {
            if (sound.second) {
                sound.second->drop();
            }
        }
        sounds.clear();

        // Liberar el motor de sonido
        soundEngine->drop();
        soundEngine = nullptr;
    }
}

bool AudioManager::playMusic(const std::string& fileName, bool loop, float volume) {
    if (!soundEngine) return false;

    // Detener m�sica anterior si existe
    if (!currentMusicName.empty()) {
        auto it = sounds.find(currentMusicName);
        if (it != sounds.end() && it->second) {
            it->second->stop();
            it->second->drop();
            sounds.erase(it);
        }
    }

    // Reproducir nueva m�sica
    ISound* music = soundEngine->play2D(fileName.c_str(), loop, true, true);

    if (!music) {
        std::cerr << "No se pudo reproducir la m�sica: " << fileName << std::endl;
        return false;
    }

    // Establecer volumen
    music->setVolume(volume * musicVolume);

    // Comenzar a reproducir
    music->setIsPaused(false);

    // Guardar referencia
    sounds[fileName] = music;
    currentMusicName = fileName;

    return true;
}

void AudioManager::playSound(const std::string& fileName, float volume) {
    if (!soundEngine || effectVolume <= 0.0f) return; // No reproducir si el motor no está disponible o el volumen de efectos es 0

    // Reproducir efecto de sonido (sin guardar referencia)
    soundEngine->play2D(fileName.c_str(), false, false, true)->setVolume(volume * effectVolume);
}

void AudioManager::pauseMusic() {
    if (!currentMusicName.empty()) {
        auto it = sounds.find(currentMusicName);
        if (it != sounds.end() && it->second) {
            it->second->setIsPaused(true);
        }
    }
}

void AudioManager::resumeMusic() {
    if (!currentMusicName.empty()) {
        auto it = sounds.find(currentMusicName);
        if (it != sounds.end() && it->second) {
            it->second->setIsPaused(false);
        }
    }
}

void AudioManager::stopMusic() {
    if (!currentMusicName.empty()) {
        auto it = sounds.find(currentMusicName);
        if (it != sounds.end() && it->second) {
            it->second->stop();
            it->second->drop();
            sounds.erase(it);
        }
        currentMusicName = "";
    }
}

void AudioManager::setMusicVolume(float volume) {
    // Limitar rango de 0.0 a 1.0
    musicVolume = (volume < 0.0f) ? 0.0f : (volume > 1.0f) ? 1.0f : volume;

    // Actualizar volumen de la m�sica actual
    if (!currentMusicName.empty()) {
        auto it = sounds.find(currentMusicName);
        if (it != sounds.end() && it->second) {
            it->second->setVolume(musicVolume);
        }
    }
}

void AudioManager::setEffectVolume(float volume) {
    // Limitar rango de 0.0 a 1.0
    effectVolume = (volume < 0.0f) ? 0.0f : (volume > 1.0f) ? 1.0f : volume;
}

AudioManager::~AudioManager() {
    close();
}