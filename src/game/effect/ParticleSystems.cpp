
#include "game/effect/ParticleSystems.h"

ParticleParams g_particleParameters[ParticleParam_MAX];


class MagicMissileExplosionParticle : public ParticleParams {
public:
	MagicMissileExplosionParticle() {
		load();
	}
	
	void load() {
		m_nbMax = 100;
		m_life = 1500;
		m_lifeRandom = 0;
		m_pos = Vec3f(10.f);
		m_direction = Vec3f(0.f, -1.f, 0.f);
		m_angle = glm::radians(360.f);
		m_speed = 130;
		m_speedRandom = 100;
		m_gravity = Vec3f(0.f, 10.f, 0.f);
		m_flash = 0;
		m_rotation = 1.0f / (101 - 16);
	
		m_startSegment.m_size = 5;
		m_startSegment.m_sizeRandom = 10;
		m_startSegment.m_color = Color(110, 110, 110, 110).to<float>();
		m_startSegment.m_colorRandom = Color(100, 100, 100, 100).to<float>();
		m_endSegment.m_size = 0;
		m_endSegment.m_sizeRandom = 2;
		m_endSegment.m_color = Color(0, 0, 120, 10).to<float>();
		m_endSegment.m_colorRandom = Color(50, 50, 50, 50).to<float>();
		
		m_texture.set("graph/particles/magicexplosion", 0, 500);
		
		m_blendMode = RenderMaterial::Additive;
		m_spawnFlags = 0;
		m_looping = false;
	}
};

class MagicMissileExplosionMrCheatParticle : public MagicMissileExplosionParticle {
public:
	MagicMissileExplosionMrCheatParticle() {
		load();
	}
	
	void load() {
		MagicMissileExplosionParticle::load();
		
		m_speed = 13;
		m_speedRandom = 10;
		m_startSegment.m_size = 20;
		m_startSegment.m_color = Color(0, 0, 0, 0).to<float>();
		m_startSegment.m_colorRandom = Color(0, 0, 0, 0).to<float>();
		m_endSegment.m_color = Color(255, 40, 120, 10).to<float>();
		m_texture.set("graph/particles/(fx)_mr", 0, 500);
	}
};

void particleParametersInit() {
	
	g_particleParameters[ParticleParam_MagicMissileExplosion]    = MagicMissileExplosionParticle();
	g_particleParameters[ParticleParam_MagicMissileExplosionMar] = MagicMissileExplosionMrCheatParticle();
	
	{
	ParticleParams cp;
	cp.m_nbMax = 350;
	cp.m_life = 800;
	cp.m_lifeRandom = 2000;
	cp.m_pos = Vec3f(100, 200, 100);
	cp.m_direction = Vec3f(0.f, -1.f, 0.f);
	cp.m_angle = glm::radians(5.f);
	cp.m_speed = 120;
	cp.m_speedRandom = 84;
	cp.m_gravity = Vec3f(0, -10, 0);
	cp.m_flash = 0;
	cp.m_rotation = 1.0f / (101 - 80);

	cp.m_startSegment.m_size = 8;
	cp.m_startSegment.m_sizeRandom = 8;
	cp.m_startSegment.m_color = Color(205, 205, 255, 245).to<float>();
	cp.m_startSegment.m_colorRandom = Color(50, 50, 0, 10).to<float>();

	cp.m_endSegment.m_size = 6;
	cp.m_endSegment.m_sizeRandom = 4;
	cp.m_endSegment.m_color = Color(20, 20, 30, 0).to<float>();
	cp.m_endSegment.m_colorRandom = Color(0, 0, 40, 0).to<float>();
	
	cp.m_blendMode = RenderMaterial::Additive;
	cp.m_texture.set("graph/particles/heal_0005", 0, 100);
	cp.m_spawnFlags = PARTICLE_CIRCULAR | PARTICLE_BORDER;
	g_particleParameters[ParticleParam_Heal] = cp;
	}
	
	{
	ParticleParams cp;
	cp.m_nbMax = 350;
	cp.m_life = 800;
	cp.m_lifeRandom = 2000;
	cp.m_pos = Vec3f(100, 200, 100);
	cp.m_direction = Vec3f(0.f, -1.f, 0.f);
	cp.m_angle = glm::radians(5.f);
	cp.m_speed = 120;
	cp.m_speedRandom = 84;
	cp.m_gravity = Vec3f(0, -10, 0);
	cp.m_flash = 0;
	cp.m_rotation = 1.0f / (101 - 80);

	cp.m_startSegment.m_size = 8;
	cp.m_startSegment.m_sizeRandom = 8;
	cp.m_startSegment.m_color = Color(105, 105, 20, 145).to<float>();
	cp.m_startSegment.m_colorRandom = Color(50, 50, 0, 10).to<float>();

	cp.m_endSegment.m_size = 6;
	cp.m_endSegment.m_sizeRandom = 4;
	cp.m_endSegment.m_color = Color(20, 20, 5, 0).to<float>();
	cp.m_endSegment.m_colorRandom = Color(40, 40, 0, 0).to<float>();

	cp.m_blendMode = RenderMaterial::Additive;
	cp.m_texture.set("graph/particles/create_food", 0, 100); //5
	cp.m_spawnFlags = PARTICLE_CIRCULAR | PARTICLE_BORDER;
	
	g_particleParameters[ParticleParam_CreateFood] = cp;
	}
	
	{
	ParticleParams cp;
	cp.m_nbMax = 350;
	cp.m_life = 800;
	cp.m_lifeRandom = 2000;
	cp.m_pos = Vec3f(100, 0, 100);
	cp.m_direction = Vec3f(0.f, -1.f, 0.f);
	cp.m_angle = glm::radians(5.f);
	cp.m_speed = 120;
	cp.m_speedRandom = 84;
	cp.m_gravity = Vec3f(0, -10, 0);
	cp.m_flash = 0;
	cp.m_rotation = 1.0f / (101 - 80);

	cp.m_startSegment.m_size = 8;//6;
	cp.m_startSegment.m_sizeRandom = 8;
	cp.m_startSegment.m_color = Color(20, 205, 20, 245).to<float>();
	cp.m_startSegment.m_colorRandom = Color(50, 50, 50, 10).to<float>();

	cp.m_endSegment.m_size = 6;
	cp.m_endSegment.m_sizeRandom = 4;
	cp.m_endSegment.m_color = Color(5, 20, 5, 0).to<float>();
	cp.m_endSegment.m_colorRandom = Color(0, 40, 0, 0).to<float>();
	cp.m_blendMode = RenderMaterial::Additive;
	cp.m_texture.set("graph/particles/cure_poison", 0, 100); //5
	cp.m_spawnFlags = PARTICLE_CIRCULAR | PARTICLE_BORDER;
	g_particleParameters[ParticleParam_CurePoison] = cp;
	}
	
	{
	ParticleParams cp = ParticleParams();
	cp.m_nbMax = 100;
	cp.m_life = 2000;
	cp.m_lifeRandom = 1000;
	cp.m_pos = Vec3f(80, 10, 80);
	cp.m_direction = Vec3f(0.f, 1.f, 0.f);
	cp.m_angle = 0;
	cp.m_speed = 0;
	cp.m_speedRandom = 0;
	cp.m_gravity = Vec3f_ZERO;
	cp.m_flash = 0;
	cp.m_rotation = 0;
	cp.m_rotationRandomDirection = false;
	cp.m_rotationRandomStart = false;

	cp.m_startSegment.m_size = 10;
	cp.m_startSegment.m_sizeRandom = 3;
	cp.m_startSegment.m_color = Color(25, 25, 25, 50).to<float>();
	cp.m_startSegment.m_colorRandom = Color(51, 51, 51, 101).to<float>();

	cp.m_endSegment.m_size = 10;
	cp.m_endSegment.m_sizeRandom = 3;
	cp.m_endSegment.m_color = Color(25, 25, 25, 50).to<float>();
	cp.m_endSegment.m_colorRandom = Color(0, 0, 0, 100).to<float>();
	cp.m_texture.m_texLoop = true;

	cp.m_blendMode = RenderMaterial::AlphaAdditive;
	cp.m_freq = 150.0f;
	cp.m_texture.set("graph/particles/firebase", 4, 100);
	cp.m_spawnFlags = 0;
	
	g_particleParameters[ParticleParam_FireFieldBase] = cp;
	}
	
	{
	ParticleParams cp = ParticleParams();
	cp.m_nbMax = 50;
	cp.m_life = 1000;
	cp.m_lifeRandom = 500;
	cp.m_pos = Vec3f(100, 10, 100);
	cp.m_direction = Vec3f(0.f, -1.f, 0.f);
	cp.m_angle = glm::radians(10.f);
	cp.m_speed = 0;
	cp.m_speedRandom = 0;
	cp.m_gravity = Vec3f_ZERO;
	cp.m_flash = 0;
	cp.m_rotation = 0;
	cp.m_rotationRandomDirection = false;
	cp.m_rotationRandomStart = false;

	cp.m_startSegment.m_size = 10;
	cp.m_startSegment.m_sizeRandom = 10;
	cp.m_startSegment.m_color = Color(40, 40, 40, 50).to<float>();
	cp.m_startSegment.m_colorRandom = Color(51, 51, 51, 100).to<float>();

	cp.m_endSegment.m_size = 10;
	cp.m_endSegment.m_sizeRandom = 10;
	cp.m_endSegment.m_color = Color(0, 0, 0, 50).to<float>();
	cp.m_endSegment.m_colorRandom = Color(0, 0, 0, 100).to<float>();
	cp.m_texture.m_texLoop = false;

	cp.m_blendMode = RenderMaterial::Additive;
	cp.m_freq = 150.0f;
	cp.m_texture.set("graph/particles/fire", 0, 500);
	cp.m_spawnFlags = 0;
	
	g_particleParameters[ParticleParam_FireFieldFlame] = cp;
	}
	
	{
	ParticleParams cp = ParticleParams();
	cp.m_nbMax = 80;
	cp.m_life = 1500;
	cp.m_lifeRandom = 500;
	cp.m_pos = Vec3f(5);
	cp.m_direction = Vec3f(0.f, 1.f, 0.f);
	cp.m_angle = glm::radians(360.f);
	cp.m_speed = 200;
	cp.m_speedRandom = 0;
	cp.m_gravity = Vec3f(0, 17, 0);
	cp.m_flash = 0;
	cp.m_rotation = 1.0f / (101 - 80);
	cp.m_rotationRandomDirection = true;
	cp.m_rotationRandomStart = true;

	cp.m_startSegment.m_size = 5;
	cp.m_startSegment.m_sizeRandom = 3;
	cp.m_startSegment.m_color = Color(0, 76, 0, 0).to<float>();
	cp.m_startSegment.m_colorRandom = Color(0, 0, 0, 150).to<float>();

	cp.m_endSegment.m_size = 30;
	cp.m_endSegment.m_sizeRandom = 5;
	cp.m_endSegment.m_color = Color(0, 0, 0, 0).to<float>();
	cp.m_endSegment.m_colorRandom = Color(0, 25, 0, 20).to<float>();

	cp.m_blendMode = RenderMaterial::AlphaAdditive;
	cp.m_freq = -1;
	cp.m_texture.set("graph/particles/big_greypouf", 0, 200);
	cp.m_spawnFlags = 0;
	cp.m_looping = false;
	g_particleParameters[ParticleParam_Poison1] = cp;
	}
	
	{
	ParticleParams cp = ParticleParams();
	cp.m_nbMax = 5;
	cp.m_life = 2000;
	cp.m_lifeRandom = 1000;
	cp.m_pos = Vec3f_ZERO;
	cp.m_direction = Vec3f(0.1f);
	cp.m_angle = 0;
	cp.m_speed = 10;
	cp.m_speedRandom = 10;
	cp.m_gravity = Vec3f_ZERO;
	cp.m_flash = 21 * (1.f/100);
	cp.m_rotation = 1.0f / (101 - 80);
	cp.m_rotationRandomDirection = true;
	cp.m_rotationRandomStart = true;

	cp.m_startSegment.m_size = 5;
	cp.m_startSegment.m_sizeRandom = 3;
	cp.m_startSegment.m_color = Color(0, 50, 0, 40).to<float>();
	cp.m_startSegment.m_colorRandom = Color(0, 100, 0, 50).to<float>();

	cp.m_endSegment.m_size = 8;
	cp.m_endSegment.m_sizeRandom = 13;
	cp.m_endSegment.m_color = Color(0, 60, 0, 40).to<float>();
	cp.m_endSegment.m_colorRandom = Color(0, 100, 0, 50).to<float>();

	cp.m_blendMode = RenderMaterial::Screen;
	cp.m_freq = -1;
	cp.m_texture.set("graph/particles/big_greypouf", 0, 200);
	cp.m_spawnFlags = 0;
	g_particleParameters[ParticleParam_Poison2] = cp;
	}
	
	{
	ParticleParams cp = ParticleParams();
	cp.m_nbMax = 100;
	cp.m_life = 500;
	cp.m_lifeRandom = 300;
	cp.m_pos = Vec3f(20, 0.f, 20);

	cp.m_direction = Vec3f(0.1f);

	cp.m_angle = glm::radians(4.f);
	cp.m_speed = 150;
	cp.m_speedRandom = 50;//15;
	cp.m_gravity = Vec3f(0, 10, 0);
	cp.m_flash = 0;
	cp.m_rotation = 1.0f / (101 - 80);
	cp.m_rotationRandomDirection = true;
	cp.m_rotationRandomStart = true;

	cp.m_startSegment.m_size = 2;
	cp.m_startSegment.m_sizeRandom = 2;
	cp.m_startSegment.m_color = Color(0, 39, 0, 100).to<float>();
	cp.m_startSegment.m_colorRandom = Color(50, 21, 0, 0).to<float>();

	cp.m_endSegment.m_size = 7;
	cp.m_endSegment.m_sizeRandom = 5;
	cp.m_endSegment.m_color = Color(0, 25, 0, 100).to<float>();
	cp.m_endSegment.m_colorRandom = Color(50, 20, 0, 0).to<float>();

	cp.m_blendMode = RenderMaterial::Screen;
	cp.m_freq = 80;
	cp.m_texture.set("graph/particles/big_greypouf", 0, 200);
	cp.m_spawnFlags = 0;
	g_particleParameters[ParticleParam_Poison3] = cp;
	}
}
