#include "SentryClient.h"

#include <cstdlib>
#include <thread>
#include <chrono>

namespace chr=std::chrono;


int main() {
	LEGO::Application::SentryClient(SENTRY_DATABASE_PARENT_PATH, false, "<version-placeholder>",
				 "<git-sha-placeholder>");
	fprintf(stderr, "Sleeping...\n");
	std::this_thread::sleep_for(chr::seconds(3));
	fprintf(stderr, "Crashing...\n");
	int*a{};
	*a = 3;
	return EXIT_SUCCESS;
}
