#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef _H_HELPERS
#define _H_HELPERS

#define _ITERATOR_DEBUG_LEVEL 0

#include <string>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <sstream>
#include <iostream>
#include <queue>
#include <deque>
#include <chrono>
#include <algorithm>
#include <limits>
#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>
#include <time.h>

#ifndef FLT_EPSILON
#	define FLT_EPSILON 1.192092896e-07F 
#endif

#define INT_MAX 2147483647

#define BIT(x) (1 << (x))

inline double Sqr( double d ) {
	return d * d;
}

inline double Sign( double d ) {
	return (d > 0) ? 1 : -1;
}

#undef min
#undef max

inline int minsgn( int a, int b ) {
	return abs( a ) < abs( b ) ? a : b;
}
inline int maxsgn( int a, int b ) {
	return abs( a ) > abs( b ) ? a : b;
}

inline double clamp( double in, double min, double max ) {
	if (in < min) return min;
	if (in > max) return max;
	return in;
}

inline double Lerp( double t, double d1, double d2 ) {
	return (1 - t)*d1 + t * d2;
}

inline double random( double min, double max ) {
	static const double RandMax = (double)RAND_MAX;
	return ((double)rand() / RandMax) * (min + (max - min));
}

inline int random( int min, int max ) {
	return (rand() % (max - min)) + min;
}

inline double random01() {
	static const double RandMax = (double)RAND_MAX;
	return rand() / RandMax;
}

inline bool IsEpsilon( double d ) {
	return abs( d ) <= FLT_EPSILON; 
}

inline double ToDegree( double rad ) {
	return rad / (2 * M_PI) * 360;
}

struct Degree {
	double degree;
	Degree( double rad ) : degree( ToDegree( rad ) ) {};
	operator double() const {
		return degree;
	}
};

namespace stdh {
	template <typename T, typename T2>
	bool has_item( const T & in, const T2 & value ) {
		return std::find( in.begin(), in.end(), value ) != in.end();
	}

	template <typename T, typename Func>
	void filter( T & in, Func inFunc ) {
		in.erase( std::remove_if( in.begin(), in.end(), inFunc ), in.end() );
	}

	template <typename T>
	void unique( T & in ) {
		in.erase( std::unique( in.begin(), in.end() ), in.end() );
	}

	template <typename T, typename T2>
	void emplace_unique( T & in, const T2 & var ) {
		if (!has_item( in, var )) in.emplace_back( var );
	}

	template <typename T, typename T2>
	int index( T & in, const T2 & var ) {
		auto it = std::find( in.begin(), in.end(), var );
		if (it != in.end()) {
			return std::distance( in.begin(), it );
		}
		return -1;
	}

	template <typename T, typename T2>
	void concat( T & first, const T2 & second ) {
		first.insert( first.end(), second.begin(), second.end() );
	}

	template <typename T>
	void remove( T & in, const T & subset ) {

	}

	template<typename T>
	void shift( T & in, int n ) {
		std::rotate( in.begin(), in.begin() - n, in.end() );
	}

	template <typename T, typename T2>
	T2 pop_back( T & in ) {
		T2 value = in.back();
		in.pop_back();
		return value;
	}

	template <typename T>
	T minmax( const T & in, const T & min, const T & max ) {
		return std::min( min, std::max( in, max ) );
	}

	template <typename T, typename T2>
	void random( T & in, int num, T2 min, T2 max ) {
		in.resize( num );
		for (T2 & value : in) {
			value = rand() / (T2)RAND_MAX * (max - min) + min;
		}
	}

	template <typename T>
	T & merge( const T & a, const T & b, T & fin ) {
		assert( a.size() == b.size() );
	}

}

typedef std::pair<int, int> ipair;


typedef std::chrono::high_resolution_clock::time_point HrcTimePoint;
typedef std::chrono::high_resolution_clock Hrc;
typedef std::chrono::milliseconds Msec;
typedef std::chrono::microseconds Microsec;

class Timer {
	HrcTimePoint Start;
	long long TotalMicro = 0;
	bool Active = false;
public:

	inline Timer() {
		//Total.count = Msec::zero;
	}
	inline void Begin() {
		Start = Hrc::now();
		Active = true;
	}
	inline void End() {
		TotalMicro += GetMicrosec();
		Active = false;
	}
	inline bool IsActive() const {
		return Active;
	}
	inline void Reset() {
		TotalMicro = 0;
	}
	inline long long GetMsec() const {
		return std::chrono::duration_cast<Msec>(Hrc::now() - Start).count();
	}
	inline long long GetMicrosec() const {
		return std::chrono::duration_cast<Microsec>(Hrc::now() - Start).count();
	}
	inline long long GetTotalMsec() const {
		return TotalMicro / 1000;
	}
	inline long long GetTotalMicrosecs() const {
		return TotalMicro;
	}
	inline float GetTotalSeconds() const {
		return GetTotalMsec() / 1000.0f;
	}
};

class Clock {
	time_t Time;
	time_t TimeEnd;
	time_t Total = 0;

public:
	inline void Begin() {
		Time = clock();
	}
	inline void End() {
		TimeEnd = clock() - Time;
		Total += TimeEnd;
	}
	inline void Reset() {
		Total = 0;
	}
	inline long GetMsec() const {
		return TimeEnd / (CLOCKS_PER_SEC * 1000);
	}
	inline long GetTotalMsec() const {
		return Total / (CLOCKS_PER_SEC * 1000);
	}
	inline float GetTotalSeconds() const {
		return (float)Total / CLOCKS_PER_SEC;
	}
};

class Perf {
public:

};


// static array
template <typename Type, int Size>
class Static {
	Type arr[Size];
	int num = 0;
public:
	Static() {

	}
	Static & operator+=( const Type & in ) {
		arr[num] = in;
		num++;
	}
	int size() const {
		return num;
	}

};

class Color {
public:
	float r, g, b, a;

	Color() {}
	Color( float _r, float _g, float _b, float _a = 1.0f ) :r( _r ), g( _g ), b( _b ), a( _a ) {};
	Color( char _r, char _g, char _b, char _a = 1 ) : r( _r / 255.0f ), g( _g / 255.0f ), b( _b / 255.0f ), a( _a / 255.0f ) {};

	Color SetRed( float _r ) const {
		return Color( _r, g, b, a );
	}
	Color SetGreen( float _g ) const {
		return Color( r, _g, b, a );
	}
	Color SetBlue( float _b ) const {
		return Color( r, g, _b, a );
	}
	Color SetAlpha( float _a ) const {
		return Color( r, g, b, _a );
	}

	float operator[]( int index ) const {
		return (&r)[index];
	}
	float & operator[]( int index ) {
		return (&r)[index];
	}

	static const Color RED;
};

const Color Color::RED( 1.0f, 0.0f, 0.0f, 1.0f );


class Vec3 {
public:
	double x, y, z;

	inline Vec3() {};
	inline Vec3( const Vec3 & in ) :x( in.x ), y( in.y ), z( in.z ) {};
	inline Vec3( double inX, double inY, double inZ ) :x( inX ), y( inY ), z( inZ ) {};

	inline Vec3 operator+( const Vec3 & in ) const {
		return Vec3( x + in.x, y + in.y, z + in.z );
	}
	inline Vec3 operator-( const Vec3 & in ) const {
		return Vec3( x - in.x, y - in.y, z - in.z );
	}
	inline Vec3 operator*( const Vec3 & in ) const {
		return Vec3( x * in.x, y * in.y, z * in.z );
	}
	inline Vec3 operator/( const Vec3 & in ) const {
		return Vec3( x / in.x, y / in.y, z / in.z );
	}
	inline Vec3 operator+( double in ) const {
		return Vec3( x + in, y + in, z + in );
	}
	inline Vec3 operator-( double in ) const {
		return Vec3( x - in, y - in, z - in );
	}
	inline Vec3 operator*( double in ) const {
		return Vec3( x * in, y * in, z * in );
	}
	inline Vec3 operator/( double in ) const {
		return Vec3( x / in, y / in, z / in );
	}
	inline Vec3 & operator+=( const Vec3 & in ) {
		x += in.x; y += in.y; z += in.z;
		return *this;
	}
	inline Vec3 & operator-=( const Vec3 & in ) {
		x -= in.x; y -= in.y; z -= in.z;
		return *this;
	}
	inline Vec3 & operator*=( const Vec3 & in ) {
		x *= in.x; y *= in.y; z *= in.z;
		return *this;
	}
	inline Vec3 & operator/=( const Vec3 & in ) {
		x /= in.x; y /= in.y; z /= in.z;
		return *this;
	}
	inline Vec3 & operator+=( double in ) {
		x += in; y += in; z += in;
		return *this;
	}
	inline Vec3 & operator-=( double in ) {
		x -= in; y -= in; z -= in;
		return *this;
	}
	inline Vec3 & operator*=( double in ) {
		x *= in; y *= in; z *= in;
		return *this;
	}
	inline Vec3 & operator/=( double in ) {
		x /= in; y /= in; z /= in;
		return *this;
	}
	inline Vec3 & operator=( const Vec3 & in ) {
		x = in.x; y = in.y; z = in.z;
		return *this;
	}
	inline Vec3 operator-() const {
		return Vec3( -x, -y, -z );
	}
	inline bool operator==( const Vec3 & in ) const {
		return x == in.x && y == in.y && z == in.z;
	}
	inline bool operator!=( const Vec3 & in ) const {
		return x != in.x || y != in.y || z != in.z;
	}
	inline double Length() const {
		return sqrt( x * x + y * y + z * z );
	}
	inline double LengthSqr() const {
		return x * x + y * y + z * z;
	}
	inline double Dot( const Vec3 & in ) const {
		return x * in.x + y * in.y + z * in.z;
	}
	inline double DotNorm( const Vec3 & in ) const {
		return Normalized().Dot( in.Normalized() );
	}
	inline Vec3 Cross( const Vec3 & in ) const {
		//(a2b3  -   a3b2,     a3b1   -   a1b3,     a1b2   -   a2b1)
		return Vec3( y*in.z - z * in.y, z*in.x - x * in.z, x*in.y - y * in.x );
	}
	inline double Dist( const Vec3 & in ) const {
		return (in - *this).Length();
	}
	inline double DistSqr( const Vec3 & in ) const {
		return (in - *this).LengthSqr();
	}
	inline double Normalize() {
		double l = Length();
		*this /= l;
		return l;
	}
	inline Vec3 Normalized() const {
		return *this / Length();
	}
	inline Vec3 Rescaled( double in ) const {
		return *this * (in / Length());
	}
	inline Vec3 Clamped( double maxLen ) const {
		double f = Length() / maxLen;
		return (f > 1.0) ? *this / f : *this;
	}
	inline Vec3 Lerped( double t, const Vec3 & to ) const {
		return (*this)*(1 - t) + to * t;
	}
	inline Vec3 Slerped( double t, const Vec3 & to ) const {
		const double theta = acos( Dot( to ) );
		//double sinTheta = sin( theta );
		//return (*this)*sin( (1 - t)*theta ) / sinTheta + to*sin( t*theta ) / sinTheta;
		return ((*this)*sin( (1 - t)*theta ) + to * sin( t*theta )) / sin( theta );
	}
	inline Vec3 Nlerped( double t, const Vec3 & to ) const {
		return Lerped( t, to ).Normalized();
	}
	inline void MakeZero() {
		x = 0; y = 0; z = 0;
	}
	inline bool IsZero() const {
		return x == 0.0 && y == 0.0 && z == 0.0;
	}
	inline bool IsEpsilon() const {
		return abs( x ) + abs( y ) + abs( z ) <= FLT_EPSILON;
	}
	inline bool IsAxis( double eps = FLT_EPSILON ) const {
		if (abs( x ) >  eps && abs( y ) <= eps && abs( z ) <= eps) return true;
		if (abs( x ) <= eps && abs( y ) >  eps && abs( z ) <= eps) return true;
		if (abs( x ) <= eps && abs( y ) <= eps && abs( z ) >  eps) return true;
		return false;
	}
	inline Vec3 SetX( double in ) const {
		return Vec3( in, y, z );
	}
	inline Vec3 SetY( double in ) const {
		return Vec3( x, in, z );
	}
	inline Vec3 SetZ( double in ) const {
		return Vec3( x, y, in );
	}
};

struct Vec2 {
	double x, y;

	inline Vec2() :x( 0 ), y( 0 ) {};
	inline Vec2( double InX, double InY ) :x( InX ), y( InY ) {};
	inline Vec2 operator-() const {
		return Vec2( -x, -y );
	}
	inline Vec2 operator-( Vec2 InV ) const {
		return Vec2( x - InV.x, y - InV.y );
	}
	inline Vec2 operator+( Vec2 InV ) const {
		return Vec2( x + InV.x, y + InV.y );
	}
	inline Vec2 operator*( double in ) const {
		return Vec2( x*in, y*in );
	}
	inline Vec2 operator/( double in ) const {
		return Vec2( x / in, y / in );
	}
	inline bool operator==( Vec2 InV ) const {
		return (x == InV.x && y == InV.y);
	}
	inline Vec2 operator+=( Vec2 InV ) {
		x += InV.x;
		y += InV.y;
		return *this;
	}
	inline Vec2 operator-=( Vec2 InV ) {
		x -= InV.x;
		y -= InV.y;
		return *this;
	}
	inline double Dist( Vec2 InV ) const {
		return (InV - *this).Len();
	}
	inline double Dist2( Vec2 InV ) const {
		return (InV - *this).Len2();
	}
	// returns angle betwwen -PI and PI
	inline double Angle( Vec2 InV ) const {
		double dot = x * InV.x + y * InV.y;
		double det = x * InV.y - y * InV.x;
		return atan2( det, dot );
	}
	// return perpendicular vector
	inline Vec2 Perp() const {
		return Vec2( -y, x );
	}
	inline Vec2 DirTo( const Vec2 & in ) const {
		return (in - *this).Normalized();
	}
	inline double Dot( Vec2 InV ) const {
		return x * InV.x + y * InV.y;
	}
	inline double DotNorm( Vec2 InV ) const {
		return Normalized().Dot( InV.Normalized() );
	}
	inline double Len2() const {
		return x * x + y * y;
	}
	inline double Len() const {
		return sqrt( x*x + y * y );
	}
	inline double Normalize() {
		double l = Len();
		x /= l; y /= l;
		return l;
	}
	inline Vec2 Normalized() const {
		return *this / Len();
	}
	inline Vec2 Rescale( double in ) const {
		double lvLen = Len();
		return Vec2( x, y ) * (in / lvLen);
	}
	inline Vec2 Lerped( double t, Vec2 to ) const {
		return (*this)*(1 - t) + to * t;
	}
	inline Vec2 Slerped( double t, Vec2 to ) const {
		//double d = Dot( to );
		double theta = acos( Dot( to ) );
		//double sinTheta = sin( theta );
		//return (*this)*sin( (1 - t)*theta ) / sinTheta + to*sin( t*theta ) / sinTheta;
		return ((*this)*sin( (1 - t)*theta ) + to * sin( t*theta )) / sin( theta );
	}
	inline Vec2 Nlerped( double t, Vec2 to ) const {
		return Lerped( t, to ).Normalized();
	}
	inline Vec2 Left( double t ) const {
		return Slerped( t, Perp() );
	}
	inline Vec2 Right( double t ) const {
		return Slerped( t, -Perp() );
	}
	inline bool IsZero() const {
		return x == 0 && y == 0;
	}
	inline bool IsEpsilon() const {
		return abs( x ) <= FLT_EPSILON && abs( y ) <= FLT_EPSILON;
	}
	inline void GetCellAt( int & OutX, int & OutY, double CellWidth ) const {
		OutX = (int)ceil( x / CellWidth ) - 1;
		OutY = (int)ceil( y / CellWidth ) - 1;
	}
	inline static Vec2 Lerp( double Factor, Vec2 From, Vec2 To ) {
		return From * (1.0f - Factor) + To * Factor;
	}
	static Vec2 CellCenter( int InX, int InY, double CellWidth ) {
		return Vec2( InX * CellWidth - CellWidth * 0.5f, InY * CellWidth - CellWidth * 0.5f );
	}

	//static const Vec2 Zero;
	//static const Vec2 UnitX;
	//static const Vec2 UnitY;
};


class Plane {
public:
	Vec3 Normal;
	double Dist;
	//int Sign;

	inline Plane() {	};
	inline Plane( Vec3 InNormal, double InDist ) :Normal( InNormal ), Dist( InDist ) {};
	inline Plane( Vec3 InPoint, Vec3 InNormal ) {

	}

	inline double DistTo( const Vec3 & InPoint ) const {
		return (Normal * Normal.Dot( InPoint )).Length();
	}

	inline double DistSqr( const Vec3 & InPoint ) const {
		return (Normal * Normal.Dot( InPoint )).LengthSqr();
	}

	inline Vec3 Project( const Vec3 & InPoint ) const {
		return InPoint - Normal * Normal.Dot( InPoint );
	}
};



struct Line2 {
	Vec2	Normal;
	double	Dist;

	inline Line2() {

	}

	inline Line2( double A, double B, double C ) {
		Normal.x = A;
		Normal.y = B;
		Dist = C;
	}

	inline Line2( const Vec2 & InNormal, double InDist ) {
		Normal = InNormal;
		Dist = InDist;
	}

	inline Line2( const Vec2 & InA, const Vec2 & InB ) {
		FromSegment( InA, InB );
	}

	inline void FromSegment( const Vec2 & InA, const Vec2 & InB ) {
		Normal.x = InA.y - InB.y;
		Normal.y = InB.x - InA.x;
		if (!Normal.IsEpsilon()) Normal = Normal.Normalized();
		Dist = InA.x*InB.y - InB.x*InA.y;
	}

	//inline void FromRay( Vec2 point, Vec2 dir ) {
	//	Normal.x = -dir.y;
	//	Normal.y = dir.x;
	//	Dist = point.x*dir.y - dir.x*point.y;
	//}

	inline void FromRay( const Vec2 & point, const Vec2 & dir ) {
		FromSegment( point, point + dir );
	}

	inline void FromOrigin( const Vec2 & dir ) {
		//Normal.x = -dir.y;
		//Normal.y = dir.x;
		//Dist = 0;
		FromRay( Vec2( 0, 0 ), dir );
	}

	inline Vec2 GetDir() const {
		return -Normal.Perp();
	}

	inline Line2 FlipSide() const {
		return Line2( -Normal, -Dist );
	}

	inline double DistToPoint( Vec2 InPoint ) const {
		//return Normal.x*InPoint.x + Normal.y*InPoint.y + Dist / sqrt( Sqr( Normal.x ) + Sqr( Normal.y ) );
		return Normal.Dot( InPoint ) + Dist;
	}

	inline int GetSign( Vec2 InPoint, double eps = -FLT_EPSILON ) const {
		return (DistToPoint( InPoint ) >= eps )? 1 : -1;
	}

	inline bool IsSameSide( Vec2 inA, Vec2 inB ) const {
		return GetSign( inA ) == GetSign( inB );
	}

	inline Vec2 Project( Vec2 point ) const {
		//return InPoint + Normal * DistToPoint( InPoint );
		//return InPoint + Normal * abs( DistToPoint( InPoint ) );
		double x = Normal.y * (Normal.y * point.x - Normal.x * point.y) - Normal.x*Dist;
		double y = Normal.x * (-Normal.y * point.x + Normal.x * point.y) - Normal.y*Dist;
		return Vec2( x, y );
	}

	inline bool IsEpsilon() const {
		return Normal.IsEpsilon();
	}
};

class Bounds {
public:
	Vec3 Min, Max;

	inline Bounds() {

	}
	inline Bounds( Vec3 InMin, Vec3 InMax ) :Min( InMin ), Max( InMax ) {

	}
};

inline void GenSphereVectors2( int numH, int numV, std::vector<Vec3> & vecs ) {
	for (int i = 0; i < numH; i++) {
		for (int j = 0; j < numV; j++) {
			Vec3 v( sin( M_PI * i / (double)numH ) * cos( 2 * M_PI * j / (double)numV ), sin( M_PI * i / (double)numH ) * sin( 2 * M_PI * j / (double)numV ), cos( M_PI * i / (double)numH ) );
			vecs.emplace_back( v );
		}
	}

	std::unique( vecs.begin(), vecs.end() );
}

inline void GenCircleVectors( int num, std::vector<Vec3> & vecs ) {
	for (int i = 0; i < num; i++) {
		vecs.emplace_back( sin( M_PI * i / (double)num ), 0, cos( M_PI * i / (double)num ) );
	}
}

inline bool IsInside( Vec3 p, Vec3 min, Vec3 max ) {
	return (p.x > min.x && p.x < max.x) && (p.y > min.y && p.y < max.y) && (p.z > min.z && p.z < max.z);
}


class Graph {
public:
	typedef std::pair<int, int> iPair;
	typedef std::set<iPair> node;

	std::vector< node > adj;

	Graph( int V ) {
		adj.resize( V );
	}
	inline void AddEdge( int u, int v, bool reverse = false, int w = 1 ) {
		//adj[u].emplace_back( v, w  );
		//if(reverse) adj[v].emplace_back( u, w );
		adj[u].emplace( v, w  );
		if(reverse) adj[v].emplace( u, w );
	}

	//inline void SetWeight( int u, int v, int w ) {
	//}

	inline static void PredcessorToPath( int node, const std::vector<int> & pred, std::vector<int> & path ) {
		path.emplace_back( node );
		while (pred[node] != -1) {
			path.emplace_back( pred[node] );
			node = pred[node];
		}
		std::reverse( path.begin(), path.end() );
	}

	inline static std::vector<int> PredcessorToPath( int node, const std::vector<int> & pred ) {
		std::vector<int> path;
		PredcessorToPath( node, pred, path );
		return path;
	}

	void ShortestPath( int src, std::vector<int> & pred, std::vector<int> & dist ) {
		// Create a priority queue to store vertices that 
		// are being preprocessed. This is weird syntax in C++. 
		// Refer below link for details of this syntax 
		// https://www.geeksforgeeks.org/implement-min-heap-using-stl/ 
		std::priority_queue< iPair, std::vector <iPair>, std::greater<iPair> > pq;

		// Create a vector for distances and initialize all 
		// distances as infinite (INF) 
		//std::vector<int> dist( (int)adj.size(), INT_MAX );
		//std::vector<int> pred( (int)adj.size(), -1 );

		dist.resize( (int)adj.size(), INT_MAX );
		pred.resize( (int)adj.size(), -1 );

		// Insert source itself in priority queue and initialize 
		// its distance as 0. 
		pq.push( std::make_pair( 0, src ) );
		dist[src] = 0;
		pred[src] = -1;

		/* Looping till priority queue becomes empty (or all
		distances are not finalized) */
		while (!pq.empty()) {

			// The first vertex in pair is the minimum distance 
			// vertex, extract it from priority queue. 
			// vertex label is stored in second of pair (it 
			// has to be done this way to keep the vertices 
			// sorted distance (distance must be first item 
			// in pair) 
			int u = pq.top().second;
			pq.pop();

			// 'i' is used to get all adjacent vertices of a vertex 
			//std::list< iPair >::iterator i;
			//for (i = adj[u].begin(); i != adj[u].end(); ++i) {
			for (auto i = adj[u].begin(); i != adj[u].end(); ++i) {
				// Get vertex label and weight of current adjacent 
				// of u. 
				int v = (*i).first;
				int weight = (*i).second;

				//  If there is shorted path to v through u. 
				if (dist[v] > dist[u] + weight) {
					// Updating distance of v 
					dist[v] = dist[u] + weight;
					pred[v] = u;
					pq.push( std::make_pair( dist[v], v ) );
				}
			}
		}
	}

	//void ShortestPath2( int src, std::vector<int> & pred, std::vector<int> & dist ) {
	//}


	// dijkstra algorithm on a 2d weight matrix
	template<int NUM_CELLS, int WIDTH>
	static void WaveFill( int index, const std::vector<int> & costs, std::vector<int> & preds, std::vector<int> & dist ) {
		dist.resize( NUM_CELLS, INT_MAX );
		preds.resize( NUM_CELLS, -1 );

		std::priority_queue< iPair, std::vector <iPair>, std::greater<iPair> > pq;

		pq.emplace( 0, index );
		dist[index] = 0;
		preds[index] = -1;

		while (!pq.empty()) {
			int u = pq.top().second;
			pq.pop();

			const int nb[] = { u - 1,u + 1,u - WIDTH,u + WIDTH };

			for (int v : nb) {
				if (v < 0 || v > NUM_CELLS-1) continue;
				if (v % WIDTH == WIDTH-1 && u % WIDTH == 0) continue;
				if (u % WIDTH == WIDTH-1 && v % WIDTH == 0) continue;
				// negative weights are considered impassable
				if (costs[v] < 0) continue;

				int d = dist[u] + costs[v];
				if (dist[v] > d) {
					dist[v] = d;
					preds[v] = u;
					pq.emplace( d, v );
				}
			}

		}

	}

	template<int NUM_CELLS, int WIDTH>
	static void WaveFill2( int index, const std::vector<int> & costs, std::vector<int> & preds, std::vector<int> & dist ) {
		dist.resize( NUM_CELLS, INT_MAX );
		preds.resize( NUM_CELLS, -1 );

		std::deque<int> q;

		bool visited[NUM_CELLS];

		memset( visited, false, sizeof( visited ) );

		visited[index] = true;
		dist[index] = 0;
		q.emplace_back( index );

		while (!q.empty()) {
			int u = q.front();
			q.pop_front();

			const int nb[] = { u - 1,u + 1,u - WIDTH,u + WIDTH };

			for (int v : nb) {
				if (visited[v]) continue;

				if (v < 0 || v > NUM_CELLS - 1) continue;
				if (v % WIDTH == WIDTH - 1 && u % WIDTH == 0) continue;
				if (u % WIDTH == WIDTH - 1 && v % WIDTH == 0) continue;

				// any nonzero value is considered impassable
				if (costs[v] != 0) continue;

				visited[v] = true;
				dist[v] = dist[u] + 1;
				preds[v] = u;

				q.emplace_back( v );
			}
		}
	}


	//template<int NUM_CELLS, int WIDTH>
	//static void WaveFill( int index, const std::vector<int> & costs, std::vector<int> & preds, std::vector<int> & dist ) {
	//	ipair stack[NUM_CELLS];
	//	int num = 0;

	//	dist.resize( NUM_CELLS, INT_MAX );
	//	preds.resize( NUM_CELLS, -1 );

	//	dist[index] = 0;

	//	//std::deque<ipair> q;

	//	stack[num] = ipair( 0, index );
	//	num++;



	//	while (num > 0) {
	//		ipair node = stack[num - 1];
	//		num--;

	//		int nb[] = { node.second - 1,node.second + 1,node.second - WIDTH,node.second + WIDTH };

	//		for (int i : nb) {
	//			if (i < 0 || i > NUM_CELLS-1) continue;
	//			if (i % WIDTH == WIDTH-1 || i % WIDTH == 0) continue;
	//			// negative weights are considered impassable
	//			if (costs[i] < 0) continue;

	//			int d = dist[node.second] + costs[i];
	//			if (dist[i] > d) {
	//				stack[num] = ipair( d, i );
	//				dist[i] = d;
	//				preds[i] = node.second;
	//				num++;
	//			}
	//		}

	//	}

	//}

	//template<int NUM_CELLS, int WIDTH>
	//static void WaveFill( int index, const std::vector<int> & costs, std::vector<int> & preds, std::vector<int> & dist ) {
	//	ipair stack[NUM_CELLS];
	//	int num = 0;

	//	dist.resize( NUM_CELLS, INT_MAX );
	//	preds.resize( NUM_CELLS, -1 );

	//	dist[index] = 0;

	//	//std::deque<ipair> q;
	//	std::deque<int> q;

	//	bool visited[NUM_CELLS];
	//	memset( visited, false, sizeof( visited ) );

	//	visited[index] = true;
	//	dist[index] = 0;
	//	q.emplace_back( index );

	//	while (!q.empty()) {
	//		//ipair node = stack[num - 1];
	//		//num--;

	//		int nb[] = { node.second - 1,node.second + 1,node.second - WIDTH,node.second + WIDTH };

	//		for (int i : nb) {
	//			if (i < 0 || i > NUM_CELLS - 1) continue;
	//			if (i % WIDTH == WIDTH - 1 || i % WIDTH == 0) continue;
	//			// negative weights are considered impassable
	//			if (costs[i] < 0) continue;

	//			int d = dist[node.second] + costs[i];
	//			if (dist[i] > d) {
	//				stack[num] = ipair( d, i );
	//				dist[i] = d;
	//				preds[i] = node.second;
	//				num++;
	//			}
	//		}

	//	}

	//}
};

#endif // ! _H_HELPERS