#include <iostream>
#include <set>
#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Postgres.h>
#include <Windows.h>

#pragma execution_character_set( "utf-8" )

class Book;

class Publisher {
public:
	std::string name;
	Wt::Dbo::collection<Wt::Dbo::ptr<Book>> books;

	template<typename Action>
	void persist(Action& a) {
		Wt::Dbo::field(a, name, "name");
		Wt::Dbo::hasMany(a, books, Wt::Dbo::ManyToOne, "publisher");
	}
};

class Stock;

class Book {
public:
	std::string title;
	Wt::Dbo::ptr<Publisher> publisher;
	Wt::Dbo::collection<Wt::Dbo::ptr<Stock>> stocks;

	template<typename Action>
	void persist(Action& a) {
		Wt::Dbo::field(a, title, "title");
		Wt::Dbo::belongsTo(a, publisher, "publisher");
		Wt::Dbo::hasMany(a, stocks, Wt::Dbo::ManyToOne, "book");
	}
};

class Shop {
public:
	std::string name;
	Wt::Dbo::collection<Wt::Dbo::ptr<Stock>> stocks;

	template<typename Action>
	void persist(Action& a) {
		Wt::Dbo::field(a, name, "name");
		Wt::Dbo::hasMany(a, stocks, Wt::Dbo::ManyToOne, "shop");
	}
};

class Sale;

class Stock {
public:
	int count;
	Wt::Dbo::ptr<Book> book;
	Wt::Dbo::ptr<Shop> shop;
	Wt::Dbo::collection < Wt::Dbo::ptr<Sale>> sales;

	template<typename Action>
	void persist(Action& a) {
		Wt::Dbo::field(a, count, "count");
		Wt::Dbo::belongsTo(a, book, "book");
		Wt::Dbo::belongsTo(a, shop, "shop");
		Wt::Dbo::hasMany(a, sales, Wt::Dbo::ManyToOne, "stock");
	}
};

class Sale {
public:
	int price;
	std::string date;
	int count;
	Wt::Dbo::ptr<Stock> stock;

	template<typename Action>
	void persist(Action& a) {
		Wt::Dbo::field(a, price, "price");
		Wt::Dbo::field(a, date, "date");
		Wt::Dbo::field(a, count, "count");
		Wt::Dbo::belongsTo(a, stock, "stock");
	}
};

int main() {
	//	setlocale(LC_ALL, "Russian");
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
	setvbuf(stdout, nullptr, _IOFBF, 1000);

	try {
		std::string connstr = "user=postgres password=postgres host=127.0.0.1 port=5432 dbname=test";
		std::unique_ptr<Wt::Dbo::backend::Postgres> connection = std::make_unique<Wt::Dbo::backend::Postgres>(connstr);

		Wt::Dbo::Session s;

		s.setConnection(std::move(connection));
		s.mapClass<Publisher>("publisher");
		s.mapClass<Book>("book");
		s.mapClass<Shop>("shop");
		s.mapClass<Shop>("stock");
		s.mapClass<Sale>("sale");

		s.dropTables();
		s.createTables();

		Wt::Dbo::Transaction t(s);

		std::unique_ptr<Publisher> pl1{new Publisher{ "Eksmo", {} }};
		std::unique_ptr<Publisher> pl2{new Publisher{ "Aist", {} }};
		std::unique_ptr<Publisher> pl3{new Publisher{ "Globus", {} }};

		auto pl1db = s.add<Publisher>(std::move(pl1));
		auto pl2db = s.add<Publisher>(std::move(pl2));
		auto pl3db = s.add<Publisher>(std::move(pl3));

		std::unique_ptr<Shop> s1{ new Shop{ "Nevsky", {} } };
		std::unique_ptr<Shop> s2{ new Shop{ "Na Dybenko", {} } };
		std::unique_ptr<Shop> s3{ new Shop{ "Liteiny", {} } };

		auto s1db = s.add<Shop>(std::move(s1));
		auto s2db = s.add<Shop>(std::move(s2));
		auto s3db = s.add<Shop>(std::move(s3));

		std::unique_ptr<Book> b1{ new Book{ "Harry Potter", pl1db, {} } };
		std::unique_ptr<Book> b2{ new Book{ "Tom Soyer", pl2db, {} } };
		std::unique_ptr<Book> b3{ new Book{ "Atlant Struggled", pl3db, {} } };

		auto b1db = s.add<Book>(std::move(b1));
		auto b2db = s.add<Book>(std::move(b2));
		auto b3db = s.add<Book>(std::move(b3));

		std::unique_ptr<Stock> st1{ new Stock{ 100, b1db, s1db, {} } };
		std::unique_ptr<Stock> st2{ new Stock{ 100, b2db, s2db, {} } };
		std::unique_ptr<Stock> st3{ new Stock{ 100, b3db, s3db, {} } };
		std::unique_ptr<Stock> st4{ new Stock{ 100, b1db, s3db, {} } };

		auto st1db = s.add<Stock>(std::move(st1));
		auto st2db = s.add<Stock>(std::move(st2));
		auto st3db = s.add<Stock>(std::move(st3));
		auto st4db = s.add<Stock>(std::move(st4));

		std::unique_ptr<Sale> sl1{ new Sale{ 20, "2024-03-25", 10, st1db}};
		std::unique_ptr<Sale> sl2{ new Sale{ 10, "2024-03-26", 10, st2db}};
		std::unique_ptr<Sale> sl3{ new Sale{ 15, "2024-03-26", 10, st3db}};

		auto sl1db = s.add<Sale>(std::move(sl1));
		auto sl2db = s.add<Sale>(std::move(sl2));
		auto sl3db = s.add<Sale>(std::move(sl3));

		t.commit();

		{
			Wt::Dbo::Transaction t2(s);
			std::string input;
			int id;
			std::cout << "Enter name or id of publisher: ";
			std::cin >> input;

			bool isNumber;

			try {
				id = std::stoi(input);
				isNumber = true;
			}
			catch (...) {
				isNumber = false;
			}

			Wt::Dbo::ptr<Publisher> pub;

			if (isNumber) {
				pub = s.find<Publisher>().where("id=?").bind(id);
			}
			else {
				pub = s.find<Publisher>().where("name=?").bind(input);
			}

			if (pub) {
				std::cout << "Publisher found: " << pub->name << std::endl;
				
				std::set<std::string> shopnames;

				for (const auto& book : pub->books) {
					for (const auto& stock : book->stocks) {
						shopnames.insert(stock->shop->name);
					}
				}
				std::cout << "Name of Shops: " << std::endl;
				for (const auto& shopname : shopnames) {
					std::cout << shopname << std::endl;
				}
			}
			else {
				std::cout << "Publisher not found" << std::endl;
			}

			t2.commit();
		}
	}
	catch (const std::exception& e) {
		std::cout << e.what();
	}
	return 0;
}
