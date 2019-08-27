#include <iostream>
#include <string>
#include <vector>
#include <set>
//#include <hash_set>
#include <unordered_set>
#include <time.h>
#include <stack>
#include <queue>
#include <tuple>
#include <fstream>

#include "../nlohmann/json.hpp"

#include "Helpers.h"

//#define DEBUG_MODE
#define DUMP 0
#define WAIT_DEBUGGER 0

#if _DEBUG || DUMP != 0 || WAIT_DEBUGGER != 0
#include "Windows.h"
#endif

#ifdef DEBUG_MODE
#	define DBG(x) x
#else
#	define DBG(x)
#	define assert(x)
#endif

using namespace std;

enum {
	NoneDir = -1,
	Left = 0,
	Up = 1,
	Right = 2,
	Down = 3
};

const char * CMD_STRING[] = { "left","up","right","down" };

const int SPEED = 5;
const int WIDTH = 30;
const int HALF_WIDTH = 15;
const int X_CELLS_COUNT = 31;
const int Y_CELLS_COUNT = 31;
const int WINDOW_WIDTH = X_CELLS_COUNT * WIDTH;
const int WINDOW_HEIGHT = Y_CELLS_COUNT * WIDTH;
const int NUM_CELLS = X_CELLS_COUNT * Y_CELLS_COUNT;
//const int MAX_INDEX = (X_CELLS_COUNT - 1) * (Y_CELLS_COUNT - 1);
const int MAX_INDEX = NUM_CELLS-1;

const int NEUTRAL_TERRITORY_SCORE = 1;
const int ENEMY_TERRITORY_SCORE = 5;
const int SAW_SCORE = 30;
const int LINE_KILL_SCORE = 50;
const int SAW_KILL_SCORE = 150;

const bool FINAL_ROUND = false;

const int MAX_EXECUTION_TIME = (FINAL_ROUND)?200:120;
const int REQUEST_MAX_TIME = (FINAL_ROUND)?9:5;
const int MAX_TICK_COUNT = (FINAL_ROUND)?2500:1500;

const int MAX_PLAYERS = FINAL_ROUND?6:6;

const int SPEED_SLOW = 3;
const int SPEED_FAST = 6;


template <typename T>
void ParseJsonArray( const nlohmann::json & json, vector<T> & out ) {
	out.reserve( json.size() );
	for (auto it = json.begin(); it != json.end(); ++it) {
		out.emplace_back( *it );
	}
}

template <typename T>
void ParseJsonArray( const nlohmann::json & json, set<T> & out ) {
	for (auto it = json.begin(); it != json.end(); ++it) {
		out.insert( *it );
	}
}

template <typename T>
void ParseJsonArray( const nlohmann::json & json, list<T> & out ) {
	for (auto it = json.begin(); it != json.end(); ++it) {
		out.insert( *it );
	}
}

int ParseDir( const string & name ) {
	if (name == "left") return Left;
	if (name == "up") return Up;
	if (name == "right") return Right;
	if (name == "down") return Down;
	return NoneDir;
}



struct Point {
	int x, y;

	inline Point() {

	}
	inline Point( const Point & in ) {
		x = in.x;
		y = in.y;
	}
	inline Point( int index ) {
		x = index % X_CELLS_COUNT * WIDTH + HALF_WIDTH;
		y = index / X_CELLS_COUNT * WIDTH + HALF_WIDTH;
	}
	inline Point( int inX, int inY ) {
		x = inX;
		y = inY;
	}

	inline bool operator<( const Point & p ) const {
		return Index() < p.Index();
	}

	inline bool operator==( const Point & p ) const {
		return x == p.x && y == p.y;
	}
	inline bool operator!=( const Point & p ) const {
		return x != p.x || y != p.y;
	}

	inline Point & operator-=( const Point & p ) {
		x -= p.x;
		y -= p.y;
		return *this;
	}

	inline Point( const nlohmann::json & json ) {
		x = json[0].get<int>();
		y = json[1].get<int>();
	}

	inline Point GetCell() const {
		return Point( x/WIDTH , y/WIDTH );
	}

	inline Point GetCellOffset() const {
		return Point( x % WIDTH - HALF_WIDTH, y % WIDTH - HALF_WIDTH );
	}

	inline bool IsOutside() const {
		return x < 0 || y < 0 || x > WINDOW_WIDTH || y > WINDOW_HEIGHT;
	}

	inline bool IsEdge() const {
		return x == HALF_WIDTH || y == HALF_WIDTH || x == WINDOW_WIDTH - HALF_WIDTH || y == WINDOW_HEIGHT - HALF_WIDTH;
	}

	inline bool IsOnCell() const {
		return (x - HALF_WIDTH) % WIDTH == 0 && (y - HALF_WIDTH) % WIDTH == 0;
	}

	inline Point & AddOffset( int dir, int offset = WIDTH ) {

		switch (dir) {
		case Left:
			x -= offset;
			break;
		case Up:
			y += offset;
			break;
		case Right:
			x += offset;
			break;
		case Down:
			y -= offset;
			break;
		}

		return *this;
	}

	inline Point GetOffset( int dir, int offset = WIDTH ) const {

		switch (dir) {
		case Left:
			return Point( x - offset, y );
		case Up:
			return Point( x, y + offset );
		case Right:
			return Point( x + offset, y );
		case Down:
			return Point( x, y - offset );
		}

	}

	inline int Index() const {
		return (x / WIDTH) + (y / WIDTH) * X_CELLS_COUNT;
	}
	//inline int Index2() const {
	//	return (x / WIDTH) + (y / WIDTH) * X_CELLS_COUNT;
	//}

	static const vector<Point> & GetCells() {
		static vector<Point> cells;

		if (!cells.empty()) {
			return cells;
		}

		cells.reserve( NUM_CELLS );

		for (int i = 0; i < X_CELLS_COUNT; i++) {
			for (int j = 0; j < Y_CELLS_COUNT; j++) {
				cells.emplace_back( i * WIDTH - HALF_WIDTH, j * WIDTH - HALF_WIDTH );
			}
		}

		return cells;
	}

};

enum {
	BonusNitro,
	BonusSlow,
	BonusSaw
};

struct Bonus {
	Point Pos;
	int Type;
	int Ticks = -1;
	bool remove = false;

	static int GetBonusType( const string & type ) {
		if (type == "n") return BonusNitro;
		if (type == "s") return BonusSlow;
		if (type == "saw") return BonusSaw;
		return -1;
	}

	Bonus( const nlohmann::json & json ) {
		Pos = json.value( "position", Point() );
		Type = GetBonusType( json["type"] );
		Ticks = json.value( "ticks", -1 );
	}

};


struct Player;
class Simulator;

class Client {
public:
	virtual int GetCommand( const Player & in, const Simulator & sim ) {
		return -1;
	}
	virtual void OnDeath( const Player & in, const Simulator & sim, int cod, int killer ) {

	}
	virtual void OnKill( const Player & in, const Simulator & sim, int killed ) {
	}
	virtual void OnCapture() {

	}
	virtual bool IsEnabled() const {
		return true;
	}
};

//Timer FillTimer2,FT3;
//int numrcalls = 0;

enum {
	T_EMPTY = 0,
	T_PLAYER = BIT(0),
	T_CAPTURED = BIT(1),
	T_TRAIL = BIT(2)
};

enum {
	COD_NONE = 0,
	COD_LINEKILL,
	COD_LINEKILL_SELF,
	COD_HEAD_ON,
	COD_HIT_WALL,
	COD_EATEN,
	COD_NO_TERRIOTRY,
	COD_SAWKILL
};

class Player {
public:

	Point Pos;
	int Dir = Left;

	int Speed = SPEED;

	int Id = -1;
	int Score = 0;

	bool IsDead = false;
	bool IsSelf = false;

	vector<Point> Trail;
	vector<Point> Territory;
	vector<Bonus> Bonuses;

	int NitroTicks = 0;
	int SlowTicks = 0;
	int SawTicks = 0;

	int TMap[NUM_CELLS];

	vector<Point> Captured;

	Client * pClient = nullptr;

	inline bool operator<( const Player & in ) const {
		return IsSelf || Id < in.Id;
	}

	Player( const nlohmann::json & json, int index ) {

		Id = index;
		IsSelf = (index == 0);

		Score = json["score"].get<int>();
		Pos = Point( json["position"] );
		Dir = !json.at("direction").is_null()?ParseDir( json["direction"] ):-1;

		ParseJsonArray( json["lines"], Trail );
		ParseJsonArray( json["territory"], Territory );
		ParseJsonArray( json["bonuses"], Bonuses );

		memset( TMap, 0, sizeof( TMap ) );
		for (const Point & p : Territory) {
			TMap[p.Index()] = T_PLAYER;
			//NumTerrritory++;
		}
		for (const Point & p : Trail) {
			TMap[p.Index()] |= T_TRAIL;
		}

		for (const Bonus & b : Bonuses) {
			AddBonus( b );
		}

		//Speed = 30;

		IsDead = false;
	}

	inline Player() {
		Trail.reserve( 100 );
		Territory.reserve( NUM_CELLS );
	}

	~Player() {
	}

	inline void AddBonus( const Bonus & b ) {
		const int ticks = 30;
		
		switch (b.Type) {
		case BonusNitro:
			NitroTicks += ticks;
			break;
		case BonusSlow:
			SlowTicks += ticks;
			break;
		case BonusSaw:
			SawTicks = 1;
			break;
		}
	}

	inline bool IsOutside() const {
		return Pos.x - HALF_WIDTH < 0 || Pos.y - HALF_WIDTH < 0 || Pos.x + HALF_WIDTH > WINDOW_WIDTH || Pos.y + HALF_WIDTH > WINDOW_HEIGHT;
	}

	inline bool IsCollide( const Player & in ) const {
		return abs( in.Pos.x - Pos.x ) < WIDTH && abs( in.Pos.y - Pos.y ) < WIDTH;
	}

	inline bool IsOnTerritory( const Point & pos ) const {
		return TMap[pos.Index()] & T_PLAYER;
	}

	inline bool IsOnTerritory() const {
		return TMap[Pos.Index()] & T_PLAYER;
	}

	inline bool IsOnTrail( const Point & pos ) const {
		return TMap[pos.Index()] & T_TRAIL;
		//return stdh::has_item( Trail, pos );
	}

	inline bool IsOnCaptured( const Point & pos ) const {
		return TMap[pos.Index()] & T_CAPTURED;
		//return stdh::has_item( Captured, pos );
	}

	inline bool IsOnEdge( const Point & pos ) const {
		if (TMap[pos.Index()] & T_PLAYER == false) return false;

		for (int i = 0; i < 4; i++) {
			if (TMap[pos.GetOffset( i ).Index()] & T_PLAYER == false) return true;
		}

		return false;
	}

	inline void RebuildTerritory() {
		Territory.clear();
		for (int i : TMap) {
			if (i & T_PLAYER) {
				Territory.emplace_back( i );
			}
		}
	}

	inline void TickBonuses() {

		Speed = SPEED;

		bool nitro = false;
		bool slow = false;
		
		if (NitroTicks) nitro = NitroTicks--;
		if (SlowTicks) slow = SlowTicks--;
		if (SawTicks) SawTicks--;
 
		if (nitro != slow) {
			Speed = (nitro) ? SPEED_FAST : SPEED_SLOW;
		}

	}

	inline void Move( const Simulator & sim ) {

		if (IsDead) return;

		bool onCell = (Pos.x - HALF_WIDTH) % WIDTH == 0 && (Pos.y - HALF_WIDTH) % WIDTH == 0;
		int cmd = -1;

		assert( pClient != nullptr );
		if (/*pClient != nullptr &&*/ onCell) {
			cmd = pClient->GetCommand( *this, sim );
		}

		if (onCell) {
			TickBonuses();
		}

		if (cmd != -1) {
			//	 if (cmd == Left && Dir != Right) Dir = Left;
			//else if (cmd == Up && Dir != Down) Dir = Up;
			//else if (cmd == Right && Dir != Left) Dir = Right;
			//else if (cmd == Down && Dir != Up) Dir = Down;
			Dir = cmd;
		}

		if (onCell && !IsOnTerritory()) {
			Trail.emplace_back( Pos );
			TMap[Pos.Index()] |= T_TRAIL;
		}

		switch (Dir) {
		case Left:
			Pos.x -= Speed;
			break;
		case Up:
			Pos.y += Speed;
			break;
		case Right:
			Pos.x += Speed;
			break;
		case Down:
			Pos.y -= Speed;
			break;
		}

	}

	const vector<Point> & FillTerritory() {
		int numToFill = NUM_CELLS;

		static int field[NUM_CELLS];

		memset( field, 0, sizeof( field ) );

		for (const Point & p : Territory) {
			field[p.Index()] = -1;
		}
		for (const Point & p : Trail) {
			field[p.Index()] = -2;
		}

		numToFill -= Territory.size();
		numToFill -= Trail.size();

		static bool doFill[100];
		int num = 1;

		//memset( doFill, false, sizeof( doFill ) );

		//stack<int> st;
		static int stack[NUM_CELLS];
		int sti = 0;

		for (int cell = 0; cell < NUM_CELLS; cell++ ) {

			if (field[cell] != 0) continue;

			//st.push( cell );
			stack[sti] = cell;
			sti++;

			doFill[num] = true;

			//while (!st.empty()) {
			while(sti > 0) {
				//int p = st.top();
				//st.pop();

				sti--;
				int p = stack[sti];

				field[p] = num;

				numToFill--;

				int nb[] = { p - 1, p + 1, p - X_CELLS_COUNT, p + X_CELLS_COUNT };

				for (int i = 0; i < 4; i++) {

					if (nb[i] < 0 || nb[i] > MAX_INDEX) {
						doFill[num] = false;
						continue;
					}

					if (field[nb[i]] != 0) continue;

					//st.push( nb[i] );
					stack[sti] = nb[i];
					sti++;
				}
			}

			//if (numToFill <= 0) break;

			num++;
		}

		//TMap.resize( NUM_CELLS, 0 );

		int numcap = 0;

		Captured.clear();

		for (int i = 0; i < NUM_CELLS; i++) {
			if (field[i] > 0 && doFill[field[i]]) {
				TMap[i] = T_CAPTURED | T_PLAYER;
				//Captured.emplace( i, true );
				Captured.emplace_back( i );
				Territory.emplace_back( i );
			}
		}

		for (const Point & p : Trail) {
			Territory.emplace_back( p.Index() );
			TMap[p.Index()] = T_CAPTURED | T_PLAYER;
		}

		Captured.insert( Captured.end(), Trail.begin(), Trail.end() );	
		Trail.clear();
		
		return Captured;
	}

	inline int RemoveTerritory( const vector<Point> & points ) {
		int numRemoved = 0;

		for (const Point & p : points) {
			const int pindex = p.Index();
			if (TMap[pindex] & T_PLAYER) {
				TMap[pindex] = 0;
				numRemoved++;
			}
		}

		RebuildTerritory();

		return numRemoved;
	}

	bool IsSawHit( const Point & p ) const {
		switch (Dir) {
		case Left:
			return p.x < Pos.x && abs( p.y - Pos.y ) < WIDTH;
		case Up:
			return p.y > Pos.y && abs( p.x - Pos.x ) < WIDTH;
		case Right:
			return p.x > Pos.x && abs( p.y - Pos.y ) < WIDTH;
		case Down:
			return p.y < Pos.y && abs( p.x - Pos.x ) < WIDTH;
		}
		return false;
	}

	void GetSawLine( vector<Point> & line ) const {
		Point pos = Pos;
		while (true) {
			if (pos.IsOutside()) {
				break;
			}
			else {
				line.emplace_back( pos );
			}
			pos.AddOffset( Dir );
		};
	}

#define SAW_FAST 0

	int SplitTerritory( const vector<Point> & line, int dir ) {
		int numRemoved = 0;

		const Point & pos = line[0];

		bool any = false;

		for (const Point & p : line) {
			if (IsOnTerritory( p )) {
				any = true;
				break;
			}
		}

#if SAW_FAST != 0
		if (!any) {
			return 0;
		}
		else {
			return 1;
		}
#else
		if (!any) return 0;

		for (int & n : TMap) {
			Point p( n );

			switch (dir) {
			case Left:
			case Right:
				if (Pos.y < pos.y) {
					if (p.y >= pos.y) {
						n &= ~(T_PLAYER | T_CAPTURED);
					}
				}
				else {
					if (p.y < pos.y) {
						n &= ~(T_PLAYER | T_CAPTURED);
					}
				}
				break;
			case Up:
			case Down:
				if (Pos.x < pos.x) {
					if (p.x >= pos.x) {
						n &= ~(T_PLAYER | T_CAPTURED);
					}
				}
				else {
					if (p.x < pos.x) {
						n &= ~(T_PLAYER | T_CAPTURED);
					}
				}
				break;
			}
		}

		if(numRemoved) RebuildTerritory();

		return numRemoved;

#endif
	}

};

Timer FillTimer;

class Simulator {
public:
	int Tick = 0;

	vector<Player> Players;
	vector<Bonus> Bonuses;

	bool HasDead = false;

	Simulator() {
		Players.reserve( MAX_PLAYERS );
		Bonuses.reserve( 3 );
	}

	inline const Player & GetSelfPlayer() const {
		return Players[0];
	}

	inline Player & GetSelfPlayer() {
		return Players[0];
	}

	inline void KillPlayer( Player & p, Player & killer, int cod ) {
		p.IsDead = true;

		assert( p.pClient );
		assert( killer.pClient );

		p.pClient->OnDeath( p, *this, cod, killer.Id );
		killer.pClient->OnKill( killer, *this, p.Id );

		//if(cod == COD_LINEKILL) killer.Score += LINE_KILL_SCORE;
		switch (cod) {
		case COD_LINEKILL:
		case COD_EATEN:
			killer.Score += LINE_KILL_SCORE;
			break;
		case COD_SAWKILL:
			killer.Score += SAW_KILL_SCORE;
		}

		HasDead = true;
	}

	ipair CheckDeath( const Player & p ) const {

		//if (p.IsDead) return false;

		// out of world
		if (p.IsOutside()) {
			return ipair(COD_HIT_WALL,-1);
		}

		// enemy hit our trail or we hit our own trail
		for (const Player & p2 : Players) {
			if (p2.Pos.IsOnCell() && p.IsOnTrail( p2.Pos )) {
				//if (p.Id != p2.Id) {
				//	p2.Score += LINE_KILL_SCORE;
				//}
				return ipair( p.Id != p2.Id?COD_LINEKILL:COD_LINEKILL_SELF,p2.Id);
			}
		}

		// all player territory was captured
		if (p.Territory.empty()) {
			return ipair(COD_NO_TERRIOTRY,-1);
		}

		// head on check
		for (const Player & p2 : Players) {
			if (p.Id == p2.Id) continue;

			if (p.IsCollide( p2 )) {
				if (p2.Trail.size() <= p.Trail.size()) return ipair(COD_HEAD_ON,p2.Id);
			}
		}

		return ipair(COD_NONE,-1);

	}

	void UseSaw( Player & p ) {
		if (p.SawTicks) {

			p.SawTicks = 0;
			
			vector<Point> sawLine;
			sawLine.reserve( X_CELLS_COUNT );

			p.GetSawLine( sawLine );
			
			for (Player & p2 : Players) {
				if (p.Id == p2.Id) continue;

				if (p.IsSawHit( p2.Pos )) {
					//p.Score += SAW_KILL_SCORE;
					//p2.IsDead = true;
					//p2.pClient->OnDeath( COD_SAWKILL, p.Id );
					KillPlayer( p2, p, COD_SAWKILL );
				}
				else {
					int numSplit = p2.SplitTerritory( sawLine, p.Dir );
					if (numSplit) p.Score += SAW_SCORE;
				}
			}
		}
	}

	inline void PickupBonuses( Player & p ) {
		if (p.IsDead) return;
		for (Bonus & b : Bonuses) {
			if (b.remove) continue;
			if (p.IsOnCaptured( b.Pos ) || p.Pos == b.Pos) {
				p.AddBonus( b );
				b.remove = true;
			}
		}
		if (p.SawTicks) {
			UseSaw( p );
		}
	}

	void TryFillTerritory( Player & p ) {
		if (p.IsDead) {
			return;
		}

		if ( !p.Trail.empty() && p.IsOnTerritory() ) {
			FillTimer.Begin();
			const vector<Point> & filled = p.FillTerritory();
			FillTimer.End();

			if (!filled.empty()) {

				int numCaptured = filled.size();
				int numRemoved = 0;

				for (Player & p2 : Players) {
					if (p.Id != p2.Id) {
						numRemoved += p2.RemoveTerritory( filled );
					}
				}

				p.Score += (numCaptured - numRemoved) * NEUTRAL_TERRITORY_SCORE;
				p.Score += (numRemoved)* ENEMY_TERRITORY_SCORE;
				
				assert( p.pClient );
				p.pClient->OnCapture();

				for (Player & p2 : Players) {
					if (p.IsOnCaptured( p2.Pos )) {
						KillPlayer( p2, p, COD_EATEN );
					}
				}
			}
		}

	}


	bool Simulate() {
		//bool hasDead = false;

		for (Player & p : Players) {
			if (p.pClient->IsEnabled() == false) p.IsDead = true;
			p.Move( *this );
		}
		for (Player & p : Players) {
			ipair death = CheckDeath( p );
			if (death.first != COD_NONE) {
				//p.IsDead = true;
				//assert( p.pClient );
				//p.pClient->OnDeath(death.first,death.second);
				//hasDead = true;
				KillPlayer( p, death.second != -1 ? Players[death.second] : p, death.first );
			}
		}
		for (Player & p : Players) {
			TryFillTerritory( p );
		}
		if (!Bonuses.empty()) {
			for (Player & p : Players) {
				PickupBonuses( p );
			}
		}

		//Filter( Bonuses, []( const Bonus & b ) { return b.remove;  } );
		if (HasDead) {
			HasDead = false;
			stdh::filter( Players, []( const Player & p ) { return p.IsDead && !p.IsSelf;  } );
		}
		
		Tick++;

		return true;
	}

	void InitTick( const nlohmann::json & json ) {

		Players.clear();
		Bonuses.clear();
		Tick = 0;

		const nlohmann::json & players = json["players"];
		const nlohmann::json & bonuses = json["bonuses"];

		Tick = json["tick_num"].get<int>();

		int id = 1;

		for (auto it = players.begin(); it != players.end(); ++it) {

			int index;
			if (it.key() != "i") {
				index = id;
				id++;
			}
			else {
				index = 0;
			}

			Players.emplace_back( *it, index );
		}
		for (auto it = bonuses.begin(); it != bonuses.end(); ++it) {
			Bonuses.emplace_back( *it );
		}

		sort( Players.begin(), Players.end() );
	}

};

const int RndDir = rand() % 4;

//typedef pair<int, vector<int>> Genom;

struct Genom {
	int first;
	vector<int> second;

	mutable int dir = -1;
	mutable int turns = 0;
	mutable int cod = COD_NONE;

	inline bool operator<( const Genom & in ) const {
		return (first == in.first)?(turns > in.turns):first < in.first;
	}
	inline bool operator>( const Genom & in ) const {
		//return first > in.first;
		return (first == in.first) ? (turns < in.turns) : first > in.first;
	}
};


inline int RandGen() {
	return rand() % 3 - 1;
}

inline int RandGen2() {
	return rand() / (double)RAND_MAX > 0.5 ? 1 : -1;
}

Genom TestGenom() {
	Genom g;
	//g.second.assign( { 0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0 } );
	g.second.assign( { 0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0 } );
	//g.second.assign( { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 } );
	return g;
}

Genom RandomGenom( int len ) {
	Genom g;
	g.second.resize( len );
	for (int i = 0; i < g.second.size(); i++) {
		g.second[i] = (random01() < 0.1) ? RandGen2() : 0;
	}
	return g;
}

Genom RandomGenom2( int len ) {
	//return TestGenom();
	Genom g;
	while (g.second.size() < len) {
		int cmd = RandGen();
		int nums = random( 0, 8 );
		g.second.emplace_back( cmd );
		while (nums) {
			g.second.emplace_back( 0 );
			nums--;
		}
	}
	g.second.resize( len );
	return g;
}

Genom MutateGenom( const Genom & a, float strength = 0.1 ) {
	Genom g;
	g.second.resize( a.second.size() );
	for (int i = 0; i < g.second.size(); i++) {
		if (random01() < strength) {
			g.second[i] = RandGen();
		}
		else {
			g.second[i] = a.second[i];
		}
	}
	return g;
}

Genom BreedGenoms( const Genom & a, const Genom & b ) {
	Genom g;
	g.second.resize( a.second.size() );
	for (int i = 0; i < g.second.size(); i++) {
		if (random01() < 0.5) {
			g.second[i] = a.second[i];
		}
		else {
			g.second[i] = b.second[i];
		}
	}
	return g;
}


inline int CmdToDir( int cmd, int basedir ) {
	int dir = basedir - cmd;
	if (dir < 0) dir += 4; else if (dir > 3) dir -= 4;
	return dir;
}

void GenomToCommands( const Genom & genom, int dir, vector<int> & commands ) {
	commands.reserve( genom.second.size() );

	for (int g : genom.second) {
		dir = CmdToDir( g, dir );
		commands.emplace_back( dir );
	}
	reverse( commands.begin(), commands.end() );
}

enum {
	PF_NONE = -1,
	PF_ATTACK,
	PF_RETREAT,
	PF_RETREAT_AND_ATTACK,
	PF_SIMPLE,
	PF_RANDOM
};

Timer PathTimer;

class Scheduled : public Client {
public:
	vector<int> Commands;
	
	Genom genom;

	vector<Point> Path;

	bool IsDead = false;
	bool IsCaptured = false;

	bool bIsEnabled = true;

	int Turn = 0;

	int CauseOfDeath = COD_NONE;
	int Killer = -1;

	int Score2 = 0;

	inline bool IsFinished() const {
		return IsDead || IsCaptured || Commands.empty();
	}

	int PathType = PF_NONE;

	inline void Init() {
		IsDead = false;
		IsCaptured = false;
		bIsEnabled = true;

		Path.clear();
		Commands.clear();
		
		Turn = 0;

		CauseOfDeath = COD_NONE;
		Killer = -1;

		Score2 = 0;

		PathType = PF_NONE;
	}

	struct PCache {
		vector<int> dists;
		vector<int> preds;

		vector<int> _costs;

		Point pos;
	};

	vector<PCache> Pathcache;

	int numCalls = 0;

	const PCache & InitPath( const Player & in ) {

		for (const PCache & pc : Pathcache) {
			if (pc.pos == in.Pos) return pc;
		}

		PCache pc;
		pc.pos = in.Pos;
		
		pc._costs.clear();
		pc._costs.resize( NUM_CELLS, 0 );

		for (const Point & p : in.Trail) {
			pc._costs[p.Index()] = INT_MAX;
		}

		pc._costs[pc.pos.Index()] = INT_MAX;

		pc.dists.clear();
		pc.preds.clear();

		numCalls++;

		Graph::WaveFill2<NUM_CELLS, X_CELLS_COUNT>( pc.pos.Index(), pc._costs, pc.preds, pc.dists );

		Pathcache.emplace_back( pc );

		return Pathcache.back();
	}

	void FindPath( const Player & in, const Simulator & sim, int type ) {
		const Player & selfPlayer = sim.GetSelfPlayer();
		const Scheduled & selfClient = *(Scheduled*)selfPlayer.pClient;

		if (in.Dir == NoneDir) {
			return;
		}

		if (type == PF_NONE) {
			return;
		}

		if (type == PF_RANDOM) {
			SetGenom( RandomGenom(selfClient.Commands.size()), in.Dir );
			return;
		}

		if (type == PF_SIMPLE) {
			return;
		}

		const PCache & pc = InitPath( in );

		const vector<int> & dists = pc.dists;
		const vector<int> & preds = pc.preds;

		//if (dists[selfPlayer.Pos.Index()] > selfPlayer.Trail.size()+8) {
		//	bIsEnabled = false;
		//	return;
		//}

		typedef vector<int> vint;

		vector<ipair> paths;
		//paths.clear();
		paths.reserve( 100 );

		bool onTerritory = in.IsOnTerritory();

		bool backwards = false;

		if (type == PF_RETREAT_AND_ATTACK) {
			type = (onTerritory) ? PF_ATTACK : PF_RETREAT;
		}
		if (type == PF_ATTACK) {
			const vector<Point> & selfTrail = selfPlayer.Trail;
			const vector<Point> & selfPath = selfClient.Path;

			int index = 0;

			for (const Point & p : selfTrail) {
				index++;
				int pindex = p.Index();
				
				if (dists[pindex] > index + 1) continue;

				paths.emplace_back( dists[pindex], pindex );
			}
			
			index = 0;

			for (const Point & p : selfPath) {
				index++;
				if (index < selfClient.Turn) continue;

				if (p.IsOutside()) {
					break;
				}
				if (selfPlayer.IsOnTerritory( p )) {
					continue;
				}

				int pindex = p.Index();
				
				if (dists[pindex] > selfPlayer.Trail.size() + index + 1) {
					continue;
				}
				
				// trail cut check
				if (dists[pindex] > index) {
					paths.emplace_back( dists[pindex], pindex );
				}

				// head on check
				else if (dists[pindex] == index) {
					const int trailLen = !in.IsOnTerritory( p )?in.Trail.size() + dists[pindex]:0;
					const int selfTrailLen = selfPlayer.Trail.size() + index;

					if (trailLen < selfTrailLen) {
						paths.emplace_back( dists[pindex], pindex );
					}
				}
			}
		}
		else if (type == PF_RETREAT) {
			for (const Point & p : in.Territory) {
				paths.emplace_back( dists[p.Index()], p.Index() );
			}
			if (in.IsOnTerritory()) backwards = true;
		}

		if (paths.empty()) {
			//bIsEnabled = false;
			return;
		}

		std::sort( paths.begin(), paths.end() );
		if (backwards) std::reverse( paths.begin(), paths.end() );

		vector<int> path;
		//path.clear();
		path.reserve( 30 );

		Graph::PredcessorToPath( paths[0].second, preds, path );

		if( path.size() < 2 ) {
			return;
		}

		for (int i = 0; i < path.size()-1; i++) {
			int diff = path[i + 1] - path[i];

			switch (diff) {
			case -1:
				Commands.emplace_back( Left );
				break;
			case 1:
				Commands.emplace_back( Right );
				break;
			case -X_CELLS_COUNT:
				Commands.emplace_back( Down );
				break;
			case X_CELLS_COUNT:
				Commands.emplace_back( Up );
				break;
			default:
				assert( false );
			}
		}

		reverse( Commands.begin(), Commands.end() );

	}

	void GetPathPoints( const Point & startPos, vector<Point> & points ) const {
		//vector<Point> points;
		points.reserve( Commands.size() );
		Point pos = startPos;

		for (int i = Commands.size() - 1; i >= 0; i--) {
			points.emplace_back( pos );
			pos.AddOffset( Commands[i] );
		}

		//return points;
	}

	void SetGenom( const Genom & genom, int startDir ) {
		int dir = startDir;

		if (dir == -1) {
			dir = RndDir;
		}

		GenomToCommands( genom, dir, Commands );
	}

	int GetCommand( const Player & in, const Simulator & sim ) {

		if (!in.IsSelf && Commands.empty() && bIsEnabled) {
			PathTimer.Begin();
			FindPath( in, sim, PathType );
			PathTimer.End();
		}

		if (Commands.empty()) return -1;

		int cmd = Commands.back();
		Commands.pop_back();

		Turn++;

		if (in.SlowTicks) Score2 -= 1;
		if (in.NitroTicks) Score2 += 1;

		return cmd;
	}

	inline void OnDeath( const Player & in, const Simulator & sim, int cod, int killer ) {
		CauseOfDeath = cod;
		Killer = killer;
		IsDead = true;
	}

	inline void OnCapture() {
		IsCaptured = true;
	}

	inline bool IsEnabled() const {
		return bIsEnabled;
	}
};

class Genetic {
public:
	const int GenomLength = 60;
	const int PopSize = 700;
	const int NumIterations = 1;

	vector<Genom> Population;

	Simulator initialState;

	vector<Scheduled*> Clients;

	int TestNum = 0;

	//DBG( int _ScoreMap[NUM_CELLS] );

	Genetic() {
		Population.reserve( PopSize );

		Clients.resize( MAX_PLAYERS );
		for (int i = 0; i < MAX_PLAYERS; i++) {
			Clients[i] = new Scheduled();
		}
	}

	~Genetic() {
		for (Client * c : Clients) {
			delete c;
		}
	}

	void BreedPopulation( int num ) {
		vector<Genom> newGenoms( num );
		for (Genom & g : newGenoms) {
			const Genom & a = Population[random( 0, Population.size() - 1 )];
			const Genom & b = Population[random( 0, Population.size() - 1 )];
			g = BreedGenoms( a, b );
		}
		Population.insert( Population.end(), newGenoms.begin(), newGenoms.end() );
	}

	void MutatePopulation( int num, float chance = 0.5, float strength = 0.1 ) {
		vector<Genom> newGenoms;
		while (newGenoms.size() < num) {
			for (const Genom & g : Population) {
				if (random01() < chance) {
					newGenoms.emplace_back( MutateGenom( g, strength ) );
				}
			}
		}
		Population.insert( Population.end(), newGenoms.begin(), newGenoms.end() );
	}

	void RandomPopulation( int num ) {
		if( num <= 0 ) return; 
		vector<Genom> newGenoms( num );
		for (Genom & g : newGenoms) {
			g = RandomGenom( GenomLength );
		}
		Population.insert( Population.end(), newGenoms.begin(), newGenoms.end() );
	}

	inline void InitPopulation() {
		Population.clear();
		RandomPopulation( PopSize );
	}

	inline void SetInitialState( const nlohmann::json & json ) {
		initialState.InitTick( json );
	}

	inline void SortPopulation() {
		std::sort( Population.begin(), Population.end(), std::greater<Genom>() );
		//std::reverse( Population.begin(), Population.end() );
	}

	int ScoreFuncOld( const Player & in, const Scheduled & client, const Simulator & sim ) const {
		int score = in.Score;
		if (in.IsDead) score -= 10000;
		return score;
	}

	int ScoreFunc( const Player & in, const Scheduled & client, const Simulator & sim ) const {
		int score = in.Score + 200/* - initialState.GetSelfPlayer().Score*/;
		//if (client.IsCaptured) {
		//	score += 10000;
		//}
		if (in.IsDead) {
			if (client.CauseOfDeath != COD_LINEKILL && client.CauseOfDeath != COD_HEAD_ON) {
				score -= 10000;
			}
			else {
				if(!client.IsCaptured) score -= 10000;
			}
		}
		for (const Point & p : client.Path) {
			if (p.IsOutside()) break;
			if (in.IsOnTerritory( p ) && !in.IsOnCaptured( p )) score -= 10;
		}
		const Point & p = client.Path.back();
		if (!p.IsOutside() && in.IsOnEdge( p )) score += 100;
		if (in.SlowTicks) {
			score -= in.SlowTicks;
		}
		if (in.NitroTicks) {
			score += in.NitroTicks;
		}
		score += client.Score2;
		return score;
	}

	int Evaluate( const Genom & genom, int enemyType ) {
		Simulator sim = initialState;

		for (Player & p : sim.Players) {
			Scheduled * client = Clients[p.Id];
			p.pClient = client;
			client->Init();
			client->PathType = enemyType;
		}

		Player & selfPlayer = sim.GetSelfPlayer();
		Scheduled & selfClient = *(Scheduled*)selfPlayer.pClient;

		int dir = (selfPlayer.Dir != -1)?selfPlayer.Dir:rand()%4;

		selfClient.SetGenom( genom, dir );
		selfClient.GetPathPoints( selfPlayer.Pos, selfClient.Path );

		while (true) {
			if (selfClient.IsFinished()) {
				break;
			}
			if (sim.Tick >= MAX_TICK_COUNT-5) {
				break;
			}

			sim.Simulate();
		}

		genom.dir = dir;
		genom.turns = selfClient.Turn;
		genom.cod = selfClient.CauseOfDeath;

		int score = ScoreFunc( selfPlayer, selfClient, sim );

		return score;
	}

	int TestGenom( const Genom & genom ) {
		int fitness = INT_MAX;
		
		//fitness = min( fitness, Evaluate( genom, PF_ATTACK ) );
		fitness = min( fitness, Evaluate( genom, PF_RETREAT ) );
		fitness = min( fitness, Evaluate( genom, PF_RETREAT_AND_ATTACK ) );

		//int numRandoms = 10;

		//for (int i = 0; i < numRandoms; i++) {
		//	fitness = min( fitness, Evaluate( genom, PF_RANDOM ) );
		//}

		return fitness;
	}

	inline void InitFitness() {
		for (Genom & g : Population) {
			g.first = INT_MAX;
		}
		for (Scheduled * c : Clients) {
			c->Pathcache.clear();
		}
	}

	int EvaluatePopulation( int enemyType ) {
		int totalFit = 0;
		TestNum = 0;
		for (Genom & g : Population) {
			//g.first = TestGenom( g );
			g.first = min( g.first, Evaluate( g, enemyType ) );
			totalFit += g.first;
			TestNum++;
		}
		return totalFit;
	}

	inline int EvaluatePopulation() {
		InitFitness();
		if (initialState.Players.size() > 1) {
			EvaluatePopulation( PF_ATTACK );
			EvaluatePopulation( PF_RETREAT );
			EvaluatePopulation( PF_RETREAT_AND_ATTACK );
		}
		else {
			EvaluatePopulation( PF_NONE );
		}
		return 0;
	}

	void UpdateGenom( const Genom & g ) {	
		Genom copy = g;
		stdh::shift( copy.second, -1 );

		for (int i : { -1, 0, 1 }) {
			copy.second.back() = i;
			Population.emplace_back( copy );
		}
	}

	void NextTick() {
		if (Population.empty()) {
			InitPopulation();
			return;
		}
		//SortPopulation();
		Genom top = GetTopGenom();
		Population.clear();
		UpdateGenom( top );
		MutatePopulation( PopSize * 1 - Population.size(), 0.33, 0.1 );
		RandomPopulation( PopSize - Population.size() );
	}

	void GenNextPopulation() {
		if (Population.empty()) {
			InitPopulation();
			return;
		}
		Population.resize( PopSize * 0.1 );
		BreedPopulation( PopSize * 0.25 );
		MutatePopulation( PopSize * 0.65, 0.2, 0.1 );
		RandomPopulation( PopSize - Population.size() );
		Population.resize( PopSize );
	}

	void IteratePopulations() {
		int num = NumIterations;
		while (num > 0) {
			GenNextPopulation();
			int totalFit = EvaluatePopulation();
			SortPopulation();
			num--;
		}
	}

	void Clear() {
		Population.clear();
	}

	inline const Genom & GetTopGenom() const {
		return Population[0];
	}

	int GetDirection() const {

	}
};

Genetic genetic;
vector<int> commands;

int numcmds = 0;

Timer TotalTime;
//Clock TotalClock;

#ifdef DEBUG_MODE
inline void to_json( nlohmann::json & j, const Color & c ) {
	j = nlohmann::json( { c.r, c.g, c.b, c.a } );
}

int ScoreMap[NUM_CELLS];

void FillScoreMap() {
	std::fill_n( ScoreMap, NUM_CELLS, -INT_MAX );

	Scheduled s;
	const Player & self = genetic.initialState.GetSelfPlayer();
	vector<Point> path;

	for (const Genom & g : genetic.Population) {
		s.SetGenom( g, g.dir );
		path.clear();
		s.GetPathPoints( self.Pos, path );

		int index = 0;
		for (const Point & p : path) {
			if (index > g.turns) break;
			if (p.IsOutside()) break;
			int & sm = ScoreMap[p.Index()];
			sm = max( sm, g.first );
			index++;
		}
	}
}

nlohmann::json GetDebugDraw() {
	FillScoreMap();

	static vector<Color> colors;
	colors.reserve( NUM_CELLS );
	colors.clear();
	
	int maxs = genetic.GetTopGenom().first;
	if (maxs == 0) maxs = 1;

	float a = 0.3f;

	for (int s : ScoreMap) {
		Color c;
		if (s >= 0) {
			if (s == maxs) {
				c = Color( 0.0f, 0.0f, 1.0f, a );
			}
			else {
				c = Color( 0.0f, 1.0f * s / (float)maxs * 0.8, 0.0f, a );
			}
		}
		else {
			c = (s != -INT_MAX) ? Color( 1.0f, 0.0f, 0.0f, a ) : Color( 0.0f, 0.0f, 0.0f, 0.0f );

		}
		colors.emplace_back( c );
	}

	return colors;
}

#endif

void FillBench() {
	const vector<Point> & cells = Point::GetCells();
	Player player;
	for (const Point & p : cells) {
		if (p.IsEdge()) player.Territory.emplace_back( p );
	}
	Timer t;
	Player player2;

	int numFills = 0;

	while (t.GetTotalMsec() < 1000) {
		t.Begin();
		player2 = player;
		player2.FillTerritory();
		numFills++;
		t.End();
	}

	cout << numFills << endl;
}

void OnTick( const nlohmann::json & json ) {
	stringstream debugJson;

	commands.clear();

	int cmd = -1;

	if (/*commands.empty()*/true) {
		genetic.SetInitialState( json );

		TotalTime.Begin();

		genetic.NextTick();
		genetic.EvaluatePopulation();
		genetic.SortPopulation();

		TotalTime.End();
		
		int ms = FillTimer.GetTotalMsec();
		int ms2 = PathTimer.GetTotalMsec();

		float totalSecs = TotalTime.GetTotalSeconds();

		auto bestGenom = genetic.GetTopGenom();
		DBG( cerr << "Tick: " << genetic.initialState.Tick << " Fitness: " << bestGenom.first << " TotalTime: " << TotalTime.GetTotalSeconds() /*<< " TotalTime2: " << TotalClock.GetTotalSeconds()*/ << endl );

		bool emergency = bestGenom.first < -1000;

		if (!emergency) {
			int dir = bestGenom.dir;
			GenomToCommands( bestGenom, dir, commands );
		}
		else {
			DBG( cerr << "!!!EMERGENCY!!!" << endl );
			genetic.Clear();
			Scheduled client;
			client.FindPath( genetic.initialState.GetSelfPlayer(), genetic.initialState, PF_RETREAT );
			commands.swap( client.Commands );
		}

		int cod[] = { 0,0,0,0,0,0,0,0 };
		//	COD_NONE = 0,
		//	COD_LINEKILL,
		//	COD_LINEKILL_SELF,
		//	COD_HEAD_ON,
		//	COD_HIT_WALL,
		//	COD_NO_TERRIOTRY,
		//	COD_SAWKILL
		for (const Genom & g : genetic.Population) {
			cod[g.cod]++;
		}
		 
		debugJson << "Fitness: " << genetic.GetTopGenom().first << " Time: " << totalSecs;
		if (emergency) debugJson << " !!!EMERGENCY!!! ";
		//debugJson << endl;
	}
	
	if (!commands.empty()) {
		cmd = commands.back();
		commands.pop_back();
	}

	nlohmann::json output;
	
	const char * cmdJson = (cmd != -1) ? CMD_STRING[cmd] : "None";

	debugJson << " Command: " << cmdJson;

	output["command"] = cmdJson;
	output["debug"] = debugJson.str();

	DBG( output["draw"] = GetDebugDraw() );

	cout << output.dump() << endl;
}

int main() {
	//FillBench2();
	//exit( 0 );

//#ifndef _DEBUG
	srand( time( NULL ) );
//#endif

	string input;

#if DUMP != 0
	std::fstream dumps;
	dumps.open( "dumps.txt", std::fstream::in | std::fstream::out );
#endif

	while (true) {

#if WAIT_DEBUGGER != 0
		if (!IsDebuggerPresent()) {
			Sleep( 100 );
			continue;
		}
#endif

//#if DUMP == 2
//		getline( dumps, input );
//#else
//		getline( cin, input );
//#endif
		getline( cin, input );
		nlohmann::json cmd = nlohmann::json::parse( input );

#if DUMP == 1
		//cerr << cmd.dump(4) << endl;
		//cerr << endl;
		dumps << cmd.dump() << endl;
		//dumps << endl;
		Sleep( 100 );
		cout << "left" << endl;
		continue;
#endif

		try {
			if (cmd.at( "type" ) == "start_game") {
				//Simulator::InitGame( cmd["command"] );
			}
			if (cmd.at( "type" ) == "tick") {
				OnTick( cmd["params"] );
			}
			if (cmd.at( "type" ) == "end_game") {
				exit( 0 );
			}
		}
		catch (nlohmann::json::exception & e) {
			cerr << e.what() << endl;
			return -1;
		}
	}


	return 0;
}