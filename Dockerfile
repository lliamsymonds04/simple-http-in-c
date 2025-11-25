# Use a base image with gcc for building
FROM gcc:latest AS builder

# Set the working directory
WORKDIR /app

# Copy the source code
COPY . .

# Build the project
RUN make

# Use a minimal base image for the final stage
FROM debian:stable-slim

# Set the working directory
WORKDIR /app

# Install necessary runtime libraries if any (e.g., glibc for gcc binaries on alpine)
# For Alpine, you might need to install glibc if your gcc build links against it dynamically.
# For simplicity, we'll assume static linking or that alpine's musl is sufficient.
# If you encounter issues, consider using a debian-slim image for the runtime or installing glibc-compat on alpine.

# Copy the built executable from the builder stage
COPY --from=builder /app/http-server .

# Copy the public directory for static assets
COPY --from=builder /app/public ./public

# Expose the port the server listens on
EXPOSE 8080

# Run the http-server executable
CMD ["./http-server"]
