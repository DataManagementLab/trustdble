## Stage 1: build frontend

# base image to build on (node:16)
FROM node:16.13.1-alpine3.13 AS frontend-build

# make /trustdble-dashboard folder the current working directory
WORKDIR /trustdble-dashboard

# copy project files from frontend to /trustdble-dashboard/frontend on the container
COPY frontend/ ./frontend/

# change to frontend directory, install all dependencies and build the frontend
RUN cd frontend && npm install && npm run build


## Stage 2: build backend/ server

# base image to build on (node:16)
FROM node:16.13.1-alpine3.13 AS server-build

# Set node environment as production
ENV NODE_ENV production

# make /root the current working directory
WORKDIR /trustdble-dashboard

# take frontend-build (stage 1) & copy dist folder into ./trustdble-dashboard/dist
COPY --from=frontend-build /trustdble-dashboard/frontend/dist ./frontend/dist

# copy files and folders 
COPY configs/ ./configs/
COPY backend/ ./backend/
COPY blockchain-monitor/ ./blockchain-monitor/
COPY trustdble-utils/ ./trustdble-utils/
COPY package.json ./
COPY lerna.json ./

# Install all dependencies
RUN npm install

CMD ["npm", "start"]